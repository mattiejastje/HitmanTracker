#include "hook.hpp"

#include <cassert>

#include "logging.hpp"
#include "mem/read_write.hpp"

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

    int32_t operator()(JumpSourceToTarget jump) { return 5; }

    int32_t operator()(JumpTargetToSource jump) { return 5; }

    int32_t operator()(TargetPointer offset) { return 4; }

    int32_t operator()(Nop nop) { return nop.repeat; }

    int32_t operator()(Zero zero) { return zero.repeat; }
};

struct GetCodeVisitor {
    int32_t source_ptr;
    int32_t target_ptr;

    Code operator()(const Code& code) { return code; }

    Code operator()(JumpSourceToTarget jump) {
        return get_jump_code(
            target_ptr + jump.target_offset - source_ptr - jump.source_offset
            - 5
        );
    };

    Code operator()(JumpTargetToSource jump) {
        return get_jump_code(
            source_ptr + jump.source_offset - target_ptr - jump.target_offset
            - 5
        );
    };

    Code operator()(TargetPointer ptr) {
        return get_code(target_ptr + ptr.offset);
    }

    Code operator()(Nop nop) { return Code(nop.repeat, 0x90); }

    Code operator()(Zero zero) { return Code(zero.repeat, 0); }
};

static int32_t get_code_size(std::vector<Assembly> assembly) {
    int32_t result{0};
    for (auto& item : assembly) {
        result += std::visit(GetCodeSizeVisitor{}, item);
    }
    return result;
}

static Code get_code(
    int32_t source_ptr, int32_t target_ptr, std::vector<Assembly> assembly
) {
    Code code{};
    for (auto& item : assembly) {
        auto part = std::visit(GetCodeVisitor{source_ptr, target_ptr}, item);
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

static AllocPtr hook_install_target_code(
    std::shared_ptr<void> handle,
    int32_t source_ptr,
    std::vector<Assembly> target_asm
) {
    auto target_code_size = get_code_size(target_asm);
    // allocate new memory in process for target code
    // TODO manage with a unique_ptr deleter
    static_assert(sizeof(int32_t) == sizeof(void*));
    logging::debug(
        "Hook: allocating {} bytes for target code", target_code_size
    );
    auto target_alloc = virtual_alloc_ex(handle, target_code_size);
    if (!target_alloc) {
        logging::error("Hook: unable to allocate memory for target code");
        return {};
    };
    // assemble target code
    auto target_code = get_code(source_ptr, target_alloc->ptr, target_asm);
    assert(target_code_size == target_code.size());
    // install target code
    logging::debug("Hook: writing target code at {:#x}", target_alloc->ptr);
    if (!write_bytes(
            handle.get(),
            target_alloc->ptr,
            target_code.data(),
            target_code.size()
        )) {
        logging::error("Hook: unable to write target code");
        return {};
    };
    return target_alloc;
}

static bool hook_install_source_code(
    void* handle,
    int32_t source_ptr,
    int32_t target_ptr,
    const std::vector<Assembly>& source_new_asm
) {
    // assemble new source code
    auto source_new_code = get_code(source_ptr, target_ptr, source_new_asm);
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
    if (!hook_check_source_code(handle.get(), source_ptr, source_orig_code)) {
        return {};
    }
    auto target_alloc
        = hook_install_target_code(handle, source_ptr, target_asm);
    if (!target_alloc) {
        return {};
    }
    if (!hook_install_source_code(
            handle.get(), source_ptr, target_alloc->ptr, source_new_asm
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