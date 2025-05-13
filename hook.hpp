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

using AssemblyCode = std::vector<Assembly>;

struct Source {
    std::shared_ptr<void> handle;
    int32_t ptr;
    Code original_code;
};

struct SourceDeleter {
    void operator()(Source* source) const;
};

using SourcePtr = std::unique_ptr<Source, SourceDeleter>;

struct Hook {
    std::shared_ptr<void> handle;
    SourcePtr source;
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
    AssemblyCode source_new_asm,
    AssemblyCode target_asm
);

int32_t get_hook_target_ptr(const HookPtr& hook);
