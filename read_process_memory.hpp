#include <windows.h>

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "handle.hpp"
#include "logger.hpp"

bool read_bytes(
    const HandlePtr& handle, uint32_t ptr, void* buffer, std::size_t size
);

uint32_t read_uint32(const HandlePtr& handle, uint32_t ptr);

float read_float(const HandlePtr& handle, uint32_t ptr);

std::string read_string(const HandlePtr& handle, uint32_t ptr, size_t size);

uint32_t find_pointer(
    const HandlePtr& handle,
    uint32_t ptr,
    const std::vector<uint32_t>& offsets
);

uint32_t read_uint32(
    const HandlePtr& handle,
    uint32_t ptr,
    const std::vector<std::uint32_t>& offsets
);

float read_float(
    const HandlePtr& handle,
    uint32_t ptr,
    const std::vector<std::uint32_t>& offsets
);

std::string read_string(
    const HandlePtr& handle,
    uint32_t ptr,
    const std::vector<std::uint32_t>& offsets,
    std::size_t size
);
