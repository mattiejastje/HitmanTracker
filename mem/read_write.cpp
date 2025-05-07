#include "read_write.hpp"

#include <spdlog/spdlog.h>

bool read_bytes(
    void *handle, int32_t ptr, void* buffer, std::size_t size
) {
    static_assert(sizeof(int32_t) == sizeof(void*));
    if (handle && ptr) {
        SIZE_T bytes_read = 0;
        if (ReadProcessMemory(
                handle,
                reinterpret_cast<void*>(ptr),
                buffer,
                size,
                &bytes_read
            )) {
            spdlog::trace("Read {} bytes at {:#x}", size, ptr);
            return true;
        }
    }
    spdlog::trace("Failed to read {} bytes at {:#x}", size, ptr);
    return false;
}

bool write_bytes(
    void *handle, int32_t ptr, void* buffer, std::size_t size
) {
    static_assert(sizeof(int32_t) == sizeof(void*));
    if (handle && ptr) {
        SIZE_T bytes_written = 0;
        if (WriteProcessMemory(
                handle,
                reinterpret_cast<void*>(ptr),
                buffer,
                size,
                &bytes_written
            )) {
            spdlog::trace("Written {} bytes at {:#x}", size, ptr);
            return true;
        }
    }
    spdlog::trace("Failed to write {} bytes at {:#x}", size, ptr);
    return false;
}

int32_t read_int32(void *handle, int32_t ptr) {
    int32_t value = 0;
    if (read_bytes(handle, ptr, &value, sizeof(value))) {
        spdlog::trace("Read int32 {}", value);
        return value;
    }
    return 0;
}

bool write_int32(void *handle, int32_t ptr, int32_t value) {
    if (write_bytes(handle, ptr, &value, sizeof(value))) {
        spdlog::trace("Written int32 {}", value);
        return true;
    }
    return false;
}

float read_float(void *handle, int32_t ptr) {
    float value = 0;
    if (read_bytes(handle, ptr, &value, 4)) {
        spdlog::trace("Read float {}", value);
        return value;
    }
    return 0;
}

std::string read_string(void *handle, int32_t ptr, size_t size) {
    auto value = std::make_unique<char[]>(size + 1);
    if (read_bytes(handle, ptr, value.get(), size)) {
        spdlog::trace("Read string {}", value.get());
        return std::string(value.get(), strnlen(value.get(), size));
    }
    return {};
}

int32_t find_pointer(
    void *handle, int32_t ptr, const std::vector<int32_t>& offsets
) {
    for (int32_t offset : offsets) {
        ptr = read_int32(handle, ptr);
        if (ptr == 0) {
            spdlog::trace("Pointer invalid");
            return 0;
        }
        if (ptr > (0xFFFFFFFF - offset)) {
            spdlog::trace("Pointer offset overflow");
            return 0;
        }
        ptr += offset;
    }
    return ptr;
}

bool read_bytes(
    void *handle,
    int32_t ptr,
    const std::vector<std::int32_t>& offsets,
    void* buffer,
    std::size_t size
) {
    return read_bytes(handle, find_pointer(handle, ptr, offsets), buffer, size);
}

int32_t read_int32(
    void *handle,
    int32_t ptr,
    const std::vector<std::int32_t>& offsets
) {
    return read_int32(handle, find_pointer(handle, ptr, offsets));
}

float read_float(
    void *handle,
    int32_t ptr,
    const std::vector<std::int32_t>& offsets
) {
    return read_float(handle, find_pointer(handle, ptr, offsets));
}

std::string read_string(
    void *handle,
    int32_t ptr,
    const std::vector<std::int32_t>& offsets,
    std::size_t size
) {
    return read_string(handle, find_pointer(handle, ptr, offsets), size);
}
