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

struct Jump {
    Label label;
};

struct Pointer {
    Label label;
};

using Assembly = std::variant<
    Code,
    Nop,
    Zero,
    Label,
    Jump,
    Pointer>;

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
