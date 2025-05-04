#include <windows.h>

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "logger.hpp"
#include "process_handle.hpp"

bool read_bytes(
    const ProcessHandlePtr& handle,
    const void* ptr,
    void* buffer,
    std::size_t size
);

uint32_t read_uint32(const ProcessHandlePtr& handle, const void* ptr);

float read_float(const ProcessHandlePtr& handle, const void* ptr);

std::string read_string(
    const ProcessHandlePtr& handle, const void* ptr, size_t size
);

const void* find_pointer(
    const ProcessHandlePtr& handle,
    const void* ptr,
    const std::vector<uint32_t>& offsets
);

uint32_t read_uint32(
    const ProcessHandlePtr& handle,
    const void* ptr,
    const std::vector<std::uint32_t>& offsets
);

float read_float(
    const ProcessHandlePtr& handle,
    const void* ptr,
    const std::vector<std::uint32_t>& offsets
);

std::string read_string(
    const ProcessHandlePtr& handle,
    const void* ptr,
    const std::vector<std::uint32_t>& offsets,
    std::size_t size
);
