#include "alloc.hpp"

#include <spdlog/spdlog.h>

void AllocDeleter::operator()(Alloc *alloc) const {
    if (alloc && alloc->ptr) {
        auto ptr = reinterpret_cast<void *>(alloc->ptr);
        if (VirtualFreeEx(alloc->handle.get(), ptr, 0, MEM_RELEASE)) {
            spdlog::debug("Freed memory at {:#x}", alloc->ptr);
        } else {
            spdlog::error("Failed to free memory at {:#x}", alloc->ptr);
        }
    }
};

AllocPtr virtual_alloc_ex(std::shared_ptr<void> handle, intptr_t size) {
    auto ptr = VirtualAllocEx(
        handle.get(),
        nullptr,
        size,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    );
    if (ptr) {
        auto alloc_ptr = reinterpret_cast<intptr_t>(ptr);
        spdlog::debug("Allocated {} bytes at {:#x}", size, alloc_ptr);
        return AllocPtr{new Alloc{handle, alloc_ptr}};
    };
    spdlog::error("Failed to allocate {} bytes", size);
    return {};
}
