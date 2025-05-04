#include "read_process_memory.hpp"

#include "logger.hpp"

bool read_bytes(
    const ProcessHandlePtr& handle,
    const void* ptr,
    void* buffer,
    std::size_t size
) {
    if (handle && ptr) {
        SIZE_T bytes_read = 0;
        if (ReadProcessMemory(handle.get(), ptr, buffer, size, &bytes_read)) {
            spdlog::trace("Read {} bytes at {}", size, ptr);
            return true;
        }
        spdlog::trace("Failed to read {} bytes at {}", size, ptr);
    }
    return false;
}

uint32_t read_uint32(const ProcessHandlePtr& handle, const void* ptr) {
    uint32_t value = 0;
    if (read_bytes(handle, ptr, &value, 4)) {
        spdlog::trace("Read uint32 {}", value);
        return value;
    }
    return 0;
}

float read_float(const ProcessHandlePtr& handle, const void* ptr) {
    float value = 0;
    if (read_bytes(handle, ptr, &value, 4)) {
        spdlog::trace("Read float {}", value);
        return value;
    }
    return 0;
}

std::string read_string(
    const ProcessHandlePtr& handle, const void* ptr, size_t size
) {
    auto value = std::make_unique<char[]>(size + 1);
    if (read_bytes(handle, ptr, value.get(), size)) {
        spdlog::trace("Read string {}", value.get());
        return value.get();
    }
    return {};
}

const void* find_pointer(
    const ProcessHandlePtr& handle,
    const void* ptr,
    const std::vector<uint32_t>& offsets
) {
    static_assert(sizeof(uint32_t) == sizeof(void*));
    uint32_t ptr_int = reinterpret_cast<uint32_t>(ptr);
    for (uint32_t offset : offsets) {
        ptr_int = read_uint32(handle, reinterpret_cast<const void*>(ptr_int));
        if (ptr_int == 0) {
            spdlog::trace("Pointer invalid");
            return nullptr;
        }
        if (ptr_int > (0xFFFFFFFF - offset)) {
            spdlog::trace("Pointer offset overflow");
            return nullptr;
        }
        ptr_int += offset;
    }
    return reinterpret_cast<const void*>(ptr_int);
}

uint32_t read_uint32(
    const ProcessHandlePtr& handle,
    const void* ptr,
    const std::vector<std::uint32_t>& offsets
) {
    return read_uint32(handle, find_pointer(handle, ptr, offsets));
}

float read_float(
    const ProcessHandlePtr& handle,
    const void* ptr,
    const std::vector<std::uint32_t>& offsets
) {
    return read_float(handle, find_pointer(handle, ptr, offsets));
}

std::string read_string(
    const ProcessHandlePtr& handle,
    const void* ptr,
    const std::vector<std::uint32_t>& offsets,
    std::size_t size
) {
    return read_string(handle, find_pointer(handle, ptr, offsets), size);
}
