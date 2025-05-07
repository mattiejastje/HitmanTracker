#include <windows.h>

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "handle.hpp"
#include <spdlog/spdlog.h>

bool read_bytes(void *handle, int32_t ptr, void *buffer, std::size_t size);

bool write_bytes(void *handle, int32_t ptr, void *buffer, std::size_t size);

int32_t read_int32(void *handle, int32_t ptr);

bool write_int32(void *handle, int32_t ptr, int32_t value);

float read_float(void *handle, int32_t ptr);

std::string read_string(void *handle, int32_t ptr, size_t size);

int32_t find_pointer(
    void *handle, int32_t ptr, const std::vector<int32_t> &offsets
);

bool read_bytes(
    void *handle,
    int32_t ptr,
    const std::vector<std::int32_t> &offsets,
    void *buffer,
    std::size_t size
);

int32_t read_int32(
    void *handle, int32_t ptr, const std::vector<std::int32_t> &offsets
);

float read_float(
    void *handle, int32_t ptr, const std::vector<std::int32_t> &offsets
);

std::string read_string(
    void *handle,
    int32_t ptr,
    const std::vector<std::int32_t> &offsets,
    std::size_t size
);
