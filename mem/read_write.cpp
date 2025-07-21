#include "read_write.hpp"

#include <windows.h>

static bool read_process_memory(
    void* handle, void* ptr, void* buffer, ptrdiff_t size
) {
    SIZE_T bytes_read = 0;
    return ReadProcessMemory(handle, ptr, buffer, size, &bytes_read);
}

static bool write_process_memory(
    void* handle, void* ptr, void* buffer, ptrdiff_t size
) {
    SIZE_T bytes_written = 0;
    return WriteProcessMemory(handle, ptr, buffer, size, &bytes_written);
}

bool read_bytes(void* handle, intptr_t ptr, void* buffer, ptrdiff_t size) {
    if (handle && ptr) {
        if (read_process_memory(
                handle, reinterpret_cast<void*>(ptr), buffer, size
            )) {
            logging::trace("Read {} bytes at {:#x}", size, ptr);
            return true;
        }
    }
    logging::trace("Failed to read {} bytes at {:#x}", size, ptr);
    return false;
}

bool write_bytes(void* handle, intptr_t ptr, void* buffer, ptrdiff_t size) {
    if (handle && ptr) {
        if (write_process_memory(
                handle, reinterpret_cast<void*>(ptr), buffer, size
            )) {
            logging::trace("Written {} bytes at {:#x}", size, ptr);
            return true;
        }
    }
    logging::trace("Failed to write {} bytes at {:#x}", size, ptr);
    return false;
}

std::optional<std::string> read_string(
    void* handle, intptr_t ptr, ptrdiff_t size
) {
    auto value = std::make_unique<char[]>(size);
    if (read_bytes(handle, ptr, value.get(), size)) {
        auto result = std::string(value.get(), strnlen(value.get(), size));
        logging::trace("Read string {}", result);
        return result;
    }
    return {};
}
