#include "read_write.hpp"

#include <windows.h>

#include "../logging.hpp"

static_assert(sizeof(int32_t) == sizeof(void*));

bool read_bytes(void* handle, int32_t ptr, void* buffer, std::size_t size) {
    if (handle && ptr) {
        SIZE_T bytes_read = 0;
        if (ReadProcessMemory(
                handle, reinterpret_cast<void*>(ptr), buffer, size, &bytes_read
            )) {
            logging::trace("Read {} bytes at {:#x}", size, ptr);
            return true;
        }
    }
    logging::trace("Failed to read {} bytes at {:#x}", size, ptr);
    return false;
}

bool write_bytes(void* handle, int32_t ptr, void* buffer, std::size_t size) {
    if (handle && ptr) {
        SIZE_T bytes_written = 0;
        if (WriteProcessMemory(
                handle,
                reinterpret_cast<void*>(ptr),
                buffer,
                size,
                &bytes_written
            )) {
            logging::trace("Written {} bytes at {:#x}", size, ptr);
            return true;
        }
    }
    logging::trace("Failed to write {} bytes at {:#x}", size, ptr);
    return false;
}

std::optional<std::string> read_string(void* handle, int32_t ptr, size_t size) {
    auto value = std::make_unique<char[]>(size);
    if (read_bytes(handle, ptr, value.get(), size)) {
        logging::trace("Read string {}", value.get());
        return std::string(value.get(), strnlen(value.get(), size));
    }
    return {};
}

std::optional<int32_t> find_pointer(
    void* handle, int32_t ptr, const std::vector<int32_t>& offsets
) {
    for (int32_t offset : offsets) {
        logging::trace("Finding [{:#x}]+{:#x}", ptr, offset);
        auto next_ptr = read<int32_t>(handle, ptr);
        if (!next_ptr) return {};
        ptr = next_ptr.value();
        if (ptr <= 0) {
            logging::trace("Pointer zero or negative");
            return {};
        }
        if (offset >= 0 && ptr > (0x7FFFFFFF - offset)) {
            logging::trace("Pointer offset overflow");
            return {};
        }
        if (offset < 0 && ptr + offset <= 0) {
            logging::trace("Pointer offset underflow");
            return {};
        }
        ptr += offset;
        logging::trace("Found {:#x}", ptr);
    }
    return ptr;
}

bool read_bytes(
    void* handle,
    int32_t ptr,
    const std::vector<int32_t>& offsets,
    void* buffer,
    std::size_t size
) {
    auto ptr_ = find_pointer(handle, ptr, offsets);
    return ptr_ ? read_bytes(handle, ptr_.value(), buffer, size) : false;
}

std::optional<std::string> read_string(
    void* handle,
    int32_t ptr,
    const std::vector<int32_t>& offsets,
    std::size_t size
) {
    auto ptr_ = find_pointer(handle, ptr, offsets);
    return ptr_ ? read_string(handle, ptr_.value(), size) : std::nullopt;
}
