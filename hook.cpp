#include "hook.hpp"

#include <spdlog/spdlog.h>

#include "contract.hpp"
#include "mem/read_write.hpp"
#include "overloaded.hpp"

static Code get_code_i8(intptr_t value) {
    APP_REQUIRE((INT8_MIN <= value) && (value <= INT8_MAX));
    return {static_cast<uint8_t>(value)};
}

static Code get_code_i32(intptr_t value) {
    APP_REQUIRE((INT32_MIN <= value) && (value <= INT32_MAX));
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
    const auto ptr = size * ((current_ptr + size - 1) / size);
    const intptr_t final_size = ptr - current_ptr;
    APP_REQUIRE(final_size >= 0);
    APP_REQUIRE(final_size < size);
    APP_REQUIRE(ptr % size == 0);
    APP_REQUIRE(current_ptr != 1 || final_size == size - 1);
    return final_size;
}

static intptr_t get_code_size(const Assembly& item, intptr_t current_ptr) {
    return std::visit(
        overloaded{
            [current_ptr](const Align& a) {
                return get_align_size(current_ptr, a.size);
            },
            [](const Code& c) { return static_cast<intptr_t>(c.size()); },
            [](const Fill& f) { return f.size; },
            [](const Jump& j) {
                return static_cast<intptr_t>(j.code.size()) + 4;
            },
            [](const JumpShort& j) {
                return static_cast<intptr_t>(j.code.size()) + 1;
            },
            [](const Label&) { return intptr_t{0}; },
            [](const Ptr&) { return intptr_t{4}; },
        },
        item
    );
}

static intptr_t get_code_size_upper_bound(const AssemblyCode& assembly) {
    return std::ranges::fold_left(
        assembly, intptr_t{0}, [](intptr_t acc, const Assembly& item) {
            // current_ptr 1 gives worst case size
            return acc + get_code_size(item, 1);
        }
    );
}

static intptr_t resolve_ptr(const Ptr& ptr, const LabelPtrs& label_ptrs) {
    return std::visit(
        overloaded{
            [&label_ptrs](const Label& l) { return label_ptrs.at(l.index); },
            [](intptr_t p) { return p; },
        },
        ptr.ptr
    );
}

static Code get_code(
    const Assembly& item, intptr_t current_ptr, const LabelPtrs& label_ptrs
) {
    const auto size = get_code_size(item, current_ptr);
    const auto next_ptr = current_ptr + size;
    const auto code = std::visit(
        overloaded{
            [&size](const Align& a) { return Code(size, a.filler); },
            [](const Code& c) { return c; },
            [](const Fill& f) { return Code(f.size, f.filler); },
            [&label_ptrs, &next_ptr](const Jump& j) {
                auto offset = resolve_ptr(j.ptr, label_ptrs) - next_ptr;
                return add_code(j.code, get_code_i32(offset));
            },
            [&label_ptrs, &next_ptr](const JumpShort& j) {
                auto offset = resolve_ptr(j.ptr, label_ptrs) - next_ptr;
                return add_code(j.code, get_code_i8(offset));
            },
            [&label_ptrs](const Label& l) {
                APP_CHECK(label_ptrs.contains(l.index));
                return Code{};
            },
            [&label_ptrs](const Ptr& p) {
                return get_code_i32(resolve_ptr(p, label_ptrs));
            },
        },
        item
    );
    APP_REQUIRE(code.size() == size);
    return code;
}

// Pass 1: Find all labels in assembly.
static void insert_label_ptrs(
    intptr_t start_ptr, const AssemblyCode& assembly, LabelPtrs& label_ptrs
) {
    spdlog::debug("Hook: calculating label pointers for {:#x}...", start_ptr);
    auto ptr = start_ptr;
    for (const auto& item : assembly) {
        if (auto* label = std::get_if<Label>(&item)) {
            label_ptrs[label->index] = ptr;
            spdlog::debug("Hook: label {} points to {:#x}", label->index, ptr);
        }
        ptr += get_code_size(item, ptr);
    }
}

// Pass 2: Emit assembly code (all labels known, so jumps can be resolved).
static Code get_code(
    intptr_t start_ptr,
    const LabelPtrs& label_ptrs,
    const AssemblyCode& assembly
) {
    spdlog::debug("Hook: emitting assembly code for {:#x}...", start_ptr);
    auto ptr = start_ptr;
    Code code{};
    for (const auto& item : assembly) {
        const auto part = get_code(item, ptr, label_ptrs);
        code.insert(code.end(), part.begin(), part.end());
        ptr += part.size();  // part.size() == get_code_size(item, ptr)
    }
    return code;
}

static bool hook_check_source_code(
    void* handle, intptr_t source_ptr, const Code& source_code_orig
) {
    spdlog::debug("Hook: verifying source at {:#x}", source_ptr);
    // check source is large enough
    if (source_code_orig.size() < 5) {
        spdlog::error("Hook: source too small");
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
        spdlog::error("Hook: cannot read source code at {:#x}", source_ptr);
        return false;
    };
    if (memcmp(
            source_code_orig_bytes.get(),
            source_code_orig.data(),
            source_code_orig.size()
        )
        != 0) {
        spdlog::error("Hook: invalid source code at {:#x}", source_ptr);
        return false;
    };
    return true;
};

void SourceHookDeleter::operator()(SourceHook* source) const {
    if (source) {
        spdlog::debug("Hook: restoring source code at {:#x}", source->ptr);
        if (!write_bytes(
                source->handle.get(),
                source->ptr,
                source->original_code.data(),
                source->original_code.size()
            )) {
            spdlog::critical(
                "Hook: failed to restore source code at {:#x}", source->ptr
            );
        }
        delete source;
    }
};

static AllocPtr hook_new_target_alloc(
    std::shared_ptr<void> handle, const AssemblyCode& target_asm
) {
    auto target_code_size = get_code_size_upper_bound(target_asm);
    spdlog::debug(
        "Hook: allocating {} bytes for target code", target_code_size
    );
    auto target_alloc = virtual_alloc_ex(handle, target_code_size);
    if (!target_alloc) {
        spdlog::error("Hook: unable to allocate memory for target code");
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
    APP_REQUIRE(target_code.size() <= target_alloc->size);
    // install target code
    spdlog::debug("Hook: writing target code at {:#x}", target_alloc->ptr);
    if (!write_bytes(
            target_alloc->handle.get(),
            target_alloc->ptr,
            target_code.data(),
            target_code.size()
        )) {
        spdlog::error("Hook: unable to write target code");
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
        spdlog::error("Hook: new source code has wrong size");
        return {};
    }
    // install new source code
    spdlog::debug("Hook: writing new source code at {:#x}", source_ptr);
    if (!write_bytes(
            handle.get(),
            source_ptr,
            source_new_code.data(),
            source_new_code.size()
        )) {
        spdlog::error("Hook: unable to write new source code");
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
    spdlog::debug("Hook: installing...");
    // allocate target memory
    auto target_alloc = hook_new_target_alloc(handle, target_asm);
    if (!target_alloc) return {};
    // calculate label pointers
    LabelPtrs label_ptrs{};
    for (const auto& source : sources)
        insert_label_ptrs(source.ptr, source.new_asm, label_ptrs);
    insert_label_ptrs(target_alloc->ptr, target_asm, label_ptrs);
    // install target code
    if (!hook_install_target_code(target_alloc, label_ptrs, target_asm))
        return {};
    // install new source code
    std::vector<SourceHookPtr> source_hooks{};
    for (const auto& source : sources) {
        auto source_hook = hook_install_source_code(
            handle, label_ptrs, source.ptr, source.original_code, source.new_asm
        );
        if (!source_hook) return {};
        source_hooks.push_back(std::move(source_hook));
    }
    return HookPtr{new Hook{
        handle,
        std::move(source_hooks),
        std::move(label_ptrs),
        std::move(target_alloc)
    }};
};

void HookDeleter::operator()(Hook* hook) const {
    if (hook) {
        spdlog::debug("Hook: uninstalling source hooks");
        hook->source_hooks.clear();
        spdlog::debug("Hook: freeing memory for target code");
        hook->target_alloc.reset();
        delete hook;
    }
}
