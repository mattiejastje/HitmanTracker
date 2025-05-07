#pragma once

#include <windows.h>

#include <memory>

struct Alloc {
    std::shared_ptr<void> handle;
    int32_t ptr;
};

struct AllocDeleter {
    void operator()(Alloc* alloc) const;
};

using AllocPtr = std::unique_ptr<Alloc, AllocDeleter>;

AllocPtr virtual_alloc_ex(std::shared_ptr<void> handle, int32_t size);