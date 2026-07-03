#pragma once

#include <windows.h>

#include <memory>

struct Alloc {
    std::shared_ptr<void> handle;
    const intptr_t ptr;
    const intptr_t size;
};

struct AllocDeleter {
    void operator()(Alloc* alloc) const;
};

using AllocPtr = std::unique_ptr<Alloc, AllocDeleter>;

AllocPtr virtual_alloc_ex(std::shared_ptr<void> handle, intptr_t size);