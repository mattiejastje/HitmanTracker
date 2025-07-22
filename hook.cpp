#include "hook.hpp"

#include <cassert>

#include "logging.hpp"
#include "mem/read_write.hpp"

static Code get_code_i8(intptr_t value) {
    assert((INT8_MIN <= value) && (value <= INT8_MAX));
    return {static_cast<uint8_t>(value)};
}

static Code get_code_i32(intptr_t value) {
    assert((INT32_MIN <= value) && (value <= INT32_MAX));
    return {
        static_cast<uint8_t>(value),
        static_cast<uint8_t>(static_cast<uint32_t>(value) >> 8),
        static_cast<uint8_t>(static_cast<uint32_t>(value) >> 16),
        static_cast<uint8_t>(static_cast<uint32_t>(value) >> 24)
    };
}

static Code add_code(const Code& code1, const Code& code2) {
    Code code{code1};
    code.insert(code.end(), code2.begin(), code2.end());
    return code;
}

static intptr_t get_align_size(intptr_t current_ptr, intptr_t size) {
    auto ptr = size * ((current_ptr + size - 1) / size);
    intptr_t final_size = ptr - current_ptr;
    assert(final_size >= 0);
    assert(final_size <= size);
    assert(ptr % size == 0);
    return final_size;
}

struct GetCodeSizeUpperBoundVisitor {
    intptr_t operator()(const Code& code) { return code.size(); }

    intptr_t operator()(const Jump& jump) { return jump.code.size() + 4; }

    intptr_t operator()(const JumpShort& jump) { return jump.code.size() + 1; }

    intptr_t operator()(const Ptr& ptr) { return 4; }

    intptr_t operator()(const Fill& fill) { return fill.size; }

    intptr_t operator()(const Align& align) {
        return align.size - 1;  // upper bound
    }

    intptr_t operator()(const Label& label) { return 0; }
};

struct GetLabelPtrsVisitor {
    intptr_t current_ptr;
    LabelPtrs& label_ptrs;

    void operator()(const Code& code) { current_ptr += code.size(); }

    void operator()(const Jump& jump) { current_ptr += jump.code.size() + 4; };

    void operator()(const JumpShort& jump) {
        current_ptr += jump.code.size() + 1;
    }

    void operator()(const Ptr& ptr) { current_ptr += 4; }

    void operator()(const Fill& fill) { current_ptr += fill.size; }

    void operator()(const Align& align) {
        auto size = get_align_size(current_ptr, align.size);
        current_ptr += size;
    }

    void operator()(const Label& label) {
        logging::debug(
            "Hook: label {} points to {:#x}", label.index, current_ptr
        );
        auto result = label_ptrs.insert({label.index, current_ptr});
        assert(result.second);  // ensure no duplicate labels
    }
};

struct GetPtrVisitor {
    const LabelPtrs& label_ptrs;

    intptr_t operator()(const Label& label) const {
        return label_ptrs.at(label.index);
    }

    intptr_t operator()(intptr_t ptr) const { return ptr; }
};

struct GetCodeVisitor {
    intptr_t current_ptr;
    const LabelPtrs& label_ptrs;

    Code operator()(const Code& code) {
        current_ptr += code.size();
        return code;
    }

    Code operator()(const Jump& jump) {
        current_ptr += jump.code.size() + 4;
        auto offset
            = std::visit(GetPtrVisitor{label_ptrs}, jump.ptr.ptr) - current_ptr;
        return add_code(jump.code, get_code_i32(offset));
    };

    Code operator()(const JumpShort& jump) {
        current_ptr += jump.code.size() + 1;
        auto offset
            = std::visit(GetPtrVisitor{label_ptrs}, jump.ptr.ptr) - current_ptr;
        return add_code(jump.code, get_code_i8(offset));
    };

    Code operator()(const Ptr& ptr) {
        current_ptr += 4;
        return get_code_i32(std::visit(GetPtrVisitor{label_ptrs}, ptr.ptr));
    }

    Code operator()(const Fill& fill) {
        current_ptr += fill.size;
        return Code(fill.size, fill.filler);
    }

    Code operator()(const Align& align) {
        auto size = get_align_size(current_ptr, align.size);
        current_ptr += size;
        return Code(size, align.filler);
    }

    Code operator()(const Label& label) {
        assert(
            label_ptrs.find(label.index) != label_ptrs.cend()
        );  // ensure label found
        return Code{};
    }
};

static intptr_t get_code_size_upper_bound(const AssemblyCode& assembly) {
    intptr_t result{0};
    GetCodeSizeUpperBoundVisitor get_code_size_upper_bound_visitor{};
    for (auto& item : assembly)
        result += std::visit(get_code_size_upper_bound_visitor, item);
    return result;
}

// Find all labels in assembly and add them to label_ptrs.
static void get_label_ptrs(
    intptr_t current_ptr, const AssemblyCode& assembly, LabelPtrs& label_ptrs
) {
    logging::debug(
        "Hook: calculating label pointers for {:#x}...", current_ptr
    );
    GetLabelPtrsVisitor find_labels_visitor{current_ptr, label_ptrs};
    for (auto& item : assembly) std::visit(find_labels_visitor, item);
}

static Code get_code(
    intptr_t current_ptr,
    const LabelPtrs& label_ptrs,
    const AssemblyCode& assembly
) {
    Code code{};
    GetCodeVisitor get_code_visitor{current_ptr, label_ptrs};
    for (auto& item : assembly) {
        auto part = std::visit(get_code_visitor, item);
        code.insert(code.end(), part.begin(), part.end());
    }
    return code;
}

static bool hook_check_source_code(
    void* handle, intptr_t source_ptr, const Code& source_code_orig
) {
    logging::debug("Hook: verifying source at {:#x}", source_ptr);
    // check source is large enough
    if (source_code_orig.size() < 5) {
        logging::error("Hook: source too small");
        return false;
    }
    // check source_ptr is valid
    auto source_code_orig_bytes
        = std::make_unique<char[]>(source_code_orig.size());
    if (!read_bytes(
            handle,
            source_ptr,
            source_code_orig_bytes.get(),
            source_code_orig.size()
        )) {
        logging::error("Hook: cannot read source code at {:#x}", source_ptr);
        return false;
    };
    if (memcmp(
            source_code_orig_bytes.get(),
            source_code_orig.data(),
            source_code_orig.size()
        )
        != 0) {
        logging::error("Hook: invalid source code at {:#x}", source_ptr);
        return false;
    };
    return true;
};

void SourceHookDeleter::operator()(SourceHook* source) const {
    if (source) {
        logging::debug("Hook: restoring source code at {:#x}", source->ptr);
        if (!write_bytes(
                source->handle.get(),
                source->ptr,
                source->original_code.data(),
                source->original_code.size()
            )) {
            logging::critical(
                "Hook: failed to restore source code at {:#x}", source->ptr
            );
        }
    }
};

static AllocPtr hook_new_target_alloc(
    std::shared_ptr<void> handle, const AssemblyCode& target_asm
) {
    auto target_code_size = get_code_size_upper_bound(target_asm);
    logging::debug(
        "Hook: allocating {} bytes for target code", target_code_size
    );
    auto target_alloc = virtual_alloc_ex(handle, target_code_size);
    if (!target_alloc) {
        logging::error("Hook: unable to allocate memory for target code");
        return {};
    };
    return target_alloc;
}

static bool hook_install_target_code(
    const AllocPtr& target_alloc,
    const LabelPtrs& label_ptrs,
    const AssemblyCode& target_asm
) {
    // assemble target code
    auto target_code = get_code(target_alloc->ptr, label_ptrs, target_asm);
    assert(target_code.size() <= get_code_size_upper_bound(target_asm));
    // install target code
    logging::debug("Hook: writing target code at {:#x}", target_alloc->ptr);
    if (!write_bytes(
            target_alloc->handle.get(),
            target_alloc->ptr,
            target_code.data(),
            target_code.size()
        )) {
        logging::error("Hook: unable to write target code");
        return false;
    };
    return true;
}

static SourceHookPtr hook_install_source_code(
    std::shared_ptr<void> handle,
    const LabelPtrs& label_ptrs,
    intptr_t source_ptr,
    const Code& source_orig_code,
    const AssemblyCode& source_new_asm
) {
    if (!hook_check_source_code(handle.get(), source_ptr, source_orig_code))
        return {};
    // assemble new source code
    auto source_new_code = get_code(source_ptr, label_ptrs, source_new_asm);
    if (source_new_code.size() != source_orig_code.size()) {
        logging::error("Hook: new source code has wrong size");
        return {};
    }
    // install new source code
    logging::debug("Hook: writing new source code at {:#x}", source_ptr);
    if (!write_bytes(
            handle.get(),
            source_ptr,
            source_new_code.data(),
            source_new_code.size()
        )) {
        logging::error("Hook: unable to write new source code");
        return {};
    };
    return SourceHookPtr(new SourceHook{handle, source_ptr, source_orig_code});
}

// TODO suspend/resume thread whilst writing...

HookPtr install_hook(
    std::shared_ptr<void> handle,
    const std::vector<Source>& sources,
    const AssemblyCode& target_asm
) {
    logging::debug("Hook: installing...");
    // allocate target memory
    auto target_alloc = hook_new_target_alloc(handle, target_asm);
    if (!target_alloc) return {};
    // calculate label pointers
    LabelPtrs label_ptrs{};
    for (const auto& source : sources)
        get_label_ptrs(source.ptr, source.new_asm, label_ptrs);
    get_label_ptrs(target_alloc->ptr, target_asm, label_ptrs);
    // install target code
    if (!hook_install_target_code(target_alloc, label_ptrs, target_asm))
        return {};
    // install new source code
    std::vector<SourceHookPtr> source_hooks{};
    for (const auto& source : sources)
        source_hooks.push_back(std::move(hook_install_source_code(
            handle, label_ptrs, source.ptr, source.original_code, source.new_asm
        )));
    return HookPtr{new Hook{
        handle,
        std::move(source_hooks),
        std::move(label_ptrs),
        std::move(target_alloc)
    }};
};

void HookDeleter::operator()(Hook* hook) const {
    if (hook) {
        logging::debug("Hook: uninstalling source hooks");
        hook->source_hooks.clear();
        logging::debug("Hook: freeing memory for target code");
        hook->target_alloc.reset();
        delete hook;
    }
}
