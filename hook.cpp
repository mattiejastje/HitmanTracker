#include "hook.hpp"

#include <cassert>
#include <unordered_map>

#include "logging.hpp"
#include "mem/read_write.hpp"

static_assert(sizeof(int32_t) == sizeof(void*));

static Code get_code(int32_t offset) {
    return {
        static_cast<uint8_t>(offset),
        static_cast<uint8_t>(static_cast<uint32_t>(offset) >> 8),
        static_cast<uint8_t>(static_cast<uint32_t>(offset) >> 16),
        static_cast<uint8_t>(static_cast<uint32_t>(offset) >> 24)
    };
}

static Code get_jump_code(int32_t offset) {
    Code code{0xE9};
    auto offset_code = get_code(offset);
    code.insert(code.end(), offset_code.begin(), offset_code.end());
    return code;
}

struct GetCodeSizeVisitor {
    int32_t operator()(const Code& code) { return code.size(); }

    int32_t operator()(Jump jump) { return 5; }

    int32_t operator()(Pointer ptr) { return 4; }

    int32_t operator()(Nop nop) { return nop.repeat; }

    int32_t operator()(Zero zero) { return zero.repeat; }

    int32_t operator()(Label label) { return 0; }
};

struct GetLabelPtrsVisitor {
    int32_t current_ptr;
    std::unordered_map<int32_t, int32_t>& label_ptrs;

    void operator()(const Code& code) { current_ptr += code.size(); }

    void operator()(const Jump& jump) { current_ptr += 5; };

    void operator()(const Pointer& ptr) { current_ptr += 4; }

    void operator()(const Nop& nop) { current_ptr += nop.repeat; }

    void operator()(const Zero& zero) { current_ptr += zero.repeat; }

    void operator()(const Label& label) {
        auto result = label_ptrs.insert({label.index, current_ptr});
        assert(!result.second);  // ensure no duplicate labels
    }
};

struct GetCodeVisitor {
    int32_t current_ptr;
    const std::unordered_map<int32_t, int32_t>& label_ptrs;

    Code operator()(const Code& code) {
        current_ptr += code.size();
        return code;
    }

    Code operator()(const Jump& jump) {
        current_ptr += 5;
        return get_jump_code(label_ptrs.at(jump.label.index) - current_ptr);
    };

    Code operator()(const Pointer& ptr) {
        current_ptr += 4;
        return get_code(label_ptrs.at(ptr.label.index));
    }

    Code operator()(const Nop& nop) {
        current_ptr += nop.repeat;
        return Code(nop.repeat, 0x90);
    }

    Code operator()(const Zero& zero) {
        current_ptr += zero.repeat;
        return Code(zero.repeat, 0);
    }

    Code operator()(const Label& label) {
        assert(
            label_ptrs.find(label.index) != label_ptrs.cend()
        );  // ensure label found
        return Code{};
    }
};

static int32_t get_code_size(const std::vector<Assembly>& assembly) {
    int32_t result{0};
    for (auto& item : assembly)
        result += std::visit(GetCodeSizeVisitor{}, item);
    return result;
}

// Find all labels in assembly and add them to label_ptrs.
static void get_label_ptrs(
    int32_t current_ptr,
    std::vector<Assembly> assembly,
    std::unordered_map<int32_t, int32_t>& label_ptrs
) {
    GetLabelPtrsVisitor find_labels_visitor{current_ptr, label_ptrs};
    for (auto& item : assembly) std::visit(find_labels_visitor, item);
}

static Code get_code(
    int32_t current_ptr,
    const std::unordered_map<int32_t, int32_t>& label_ptrs,
    const std::vector<Assembly>& assembly
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
    void* handle, int32_t source_ptr, Code source_code_orig
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

static bool hook_restore_source_code(
    void* handle, int32_t source_ptr, Code source_code_orig
) {
    logging::debug("Hook: restoring source code at {:#x}", source_ptr);
    if (!write_bytes(
            handle, source_ptr, source_code_orig.data(), source_code_orig.size()
        )) {
        logging::error(
            "Hook: failed to restore source code at {:#x}", source_ptr
        );
        return false;
    };
    return true;
};

static AllocPtr hook_new_target_alloc(
    std::shared_ptr<void> handle, const std::vector<Assembly>& target_asm
) {
    auto target_code_size = get_code_size(target_asm);
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
    std::unordered_map<int32_t, int32_t> label_ptrs,
    int32_t source_ptr,
    std::vector<Assembly> target_asm
) {
    // assemble target code
    auto target_code = get_code(target_alloc->ptr, label_ptrs, target_asm);
    assert(get_code_size(target_asm) == target_code.size());
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

static bool hook_install_source_code(
    void* handle,
    std::unordered_map<int32_t, int32_t> label_ptrs,
    int32_t source_ptr,
    int32_t target_ptr,
    const std::vector<Assembly>& source_new_asm
) {
    // assemble new source code
    auto source_new_code = get_code(source_ptr, label_ptrs, source_new_asm);
    // install new source code
    logging::debug("Hook: writing new source code at {:#x}", source_ptr);
    if (!write_bytes(
            handle, source_ptr, source_new_code.data(), source_new_code.size()
        )) {
        logging::error("Hook: unable to write new source code");
        return false;
    };
    return true;
}

// TODO suspend/resume thread whilst writing...

HookPtr install_hook(
    std::shared_ptr<void> handle,
    int32_t source_ptr,
    Code source_orig_code,
    std::vector<Assembly> source_new_asm,
    std::vector<Assembly> target_asm
) {
    logging::debug("Hook: installing at {:#x}", source_ptr);
    // check original source code
    if (!hook_check_source_code(handle.get(), source_ptr, source_orig_code)) {
        return {};
    }
    // allocate target memory
    auto target_alloc = hook_new_target_alloc(handle, target_asm);
    if (!target_alloc) return {};
    // calculate label pointers
    std::unordered_map<int32_t, int32_t> label_ptrs{};
    get_label_ptrs(source_ptr, source_new_asm, label_ptrs);
    get_label_ptrs(target_alloc->ptr, target_asm, label_ptrs);
    // install target code
    if (!hook_install_target_code(
            target_alloc, label_ptrs, source_ptr, target_asm
        )) {
        return {};
    };
    // install new source code
    if (!hook_install_source_code(
            handle.get(),
            label_ptrs,
            source_ptr,
            target_alloc->ptr,
            source_new_asm
        )) {
        return {};
    }
    return HookPtr{
        new Hook{handle, source_ptr, source_orig_code, std::move(target_alloc)}
    };
};

void HookDeleter::operator()(Hook* hook) const {
    if (hook) {
        logging::debug("Hook: uninstalling at {:#x}", hook->source_ptr);
        hook_restore_source_code(
            hook->handle.get(), hook->source_ptr, hook->source_orig_code
        );
        logging::debug("Hook: freeing memory for target code");
        hook->target_alloc.reset();
        delete hook;
    }
}

int32_t get_hook_target_ptr(const HookPtr& hook) {
    return hook ? (hook->target_alloc ? hook->target_alloc->ptr : 0) : 0;
}