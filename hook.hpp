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

struct Label {
    int32_t index;
};

struct JumpSourceToTarget {
    Label label;
    int32_t source_offset;  // offset relative to source_ptr of jmp opcode
    int32_t target_offset;  // offset relative to target_ptr of jmp target
};

struct JumpTargetToSource {
    Label label;
    int32_t target_offset;  // offset relative to target_ptr of jmp opcode
    int32_t source_offset;  // offset relative to source_ptr of jmp target
};

struct TargetPointer {
    Label label;
    int32_t offset;  // offset relative to target_ptr of absolute pointer
};

using Assembly = std::variant<
    Code,
    Nop,
    Zero,
    Label,
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

HookPtr install_hook(
    std::shared_ptr<void> handle,
    int32_t source_ptr,
    Code source_orig_code,
    std::vector<Assembly> source_new_asm,
    std::vector<Assembly> target_asm
);

int32_t get_hook_target_ptr(const HookPtr& hook);
