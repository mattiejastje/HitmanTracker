#pragma once

#include <cstdint>
#include <memory>
#include <variant>
#include <vector>

#include "label_ptrs.hpp"
#include "mem/alloc.hpp"
#include "mem/handle.hpp"

using Code = std::vector<uint8_t>;

struct Align {
    int32_t size;
    uint8_t filler{0};
};

struct Fill {
    int32_t size;
    uint8_t filler{0};
};

struct Label {
    int32_t index;
};

struct Ptr {
    std::variant<Label, int32_t> ptr;
};

struct Jump {
    Code code;
    Ptr ptr;
};

struct JumpShort {
    Code code;
    Ptr ptr;
};

using Assembly = std::variant<Code, Align, Fill, Label, Jump, JumpShort, Ptr>;

using AssemblyCode = std::vector<Assembly>;

struct SourceHook {
    std::shared_ptr<void> handle;
    int32_t ptr;
    Code original_code;
};

struct SourceHookDeleter {
    void operator()(SourceHook* source) const;
};

using SourceHookPtr = std::unique_ptr<SourceHook, SourceHookDeleter>;

struct Hook {
    std::shared_ptr<void> handle;
    std::vector<SourceHookPtr> source_hooks;
    LabelPtrs label_ptrs;
    AllocPtr target_alloc;
};

struct HookDeleter {
    void operator()(Hook* hook) const;
};

using HookPtr = std::unique_ptr<Hook, HookDeleter>;

struct Source {
    int32_t ptr;
    Code original_code;
    AssemblyCode new_asm;
};

HookPtr install_hook(
    std::shared_ptr<void> handle,
    const std::vector<Source>& sources,
    const AssemblyCode& target_asm
);
