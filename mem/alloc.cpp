#include "alloc.hpp"

#include "../logger.hpp"

void AllocDeleter::operator()(Alloc *alloc) const {
    if (alloc && alloc->ptr) {
        auto ptr = reinterpret_cast<void *>(alloc->ptr);
        if (VirtualFreeEx(alloc->handle.get(), ptr, 0, MEM_RELEASE)) {
            logging::trace("Freed memory at {:#x}", alloc->ptr);
        } else {
            logging::error("Failed to free memory at {:#x}", alloc->ptr);
        }
    }
};

AllocPtr virtual_alloc_ex(std::shared_ptr<void> handle, int32_t size) {
    auto ptr = VirtualAllocEx(
        handle.get(),
        nullptr,
        size,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    );
    if (ptr) {
        auto alloc_ptr = reinterpret_cast<int32_t>(ptr);
        logging::trace("Allocated {} bytes at {:#x}", size, alloc_ptr);
        return AllocPtr{new Alloc{handle, alloc_ptr}};
    };
    logging::error("Failed to allocate {} bytes", size);
    return {};
}
