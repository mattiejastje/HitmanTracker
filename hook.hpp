#pragma once

#include <cstdint>
#include <memory>
#include <variant>
#include <vector>

#include "mem/alloc.hpp"
#include "mem/handle.hpp"

using Code = std::vector<uint8_t>;

struct Nop {
    int32_t repeat;
};

struct Zero {
    int32_t repeat;
};

struct JumpSourceToTarget {
    int32_t source_offset;  // offset relative to source_ptr of jmp opcode
    int32_t target_offset;  // offset relative to target_ptr of jmp target
};

struct JumpTargetToSource {
    int32_t target_offset;  // offset relative to target_ptr of jmp opcode
    int32_t source_offset;  // offset relative to source_ptr of jmp target
};

struct TargetPointer {
    int32_t offset;  // offset relative to target_ptr of absolute pointer
};

using Assembly = std::variant<
    Code,
    Nop,
    Zero,
    JumpSourceToTarget,
    JumpTargetToSource,
    TargetPointer>;

struct Hook {
    std::shared_ptr<void> handle;
    int32_t source_ptr;
    Code source_orig_code;
    AllocPtr target_alloc;
};

struct HookDeleter {
    void operator()(Hook* hook) const;
};

using HookPtr = std::unique_ptr<Hook, HookDeleter>;

HookPtr hook_install(
    std::shared_ptr<void> handle,
    int32_t source_ptr,
    Code source_orig_code,
    std::vector<Assembly> source_new_asm,
    std::vector<Assembly> target_asm
);