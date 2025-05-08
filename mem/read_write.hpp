#include <spdlog/spdlog.h>
#include <windows.h>

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "handle.hpp"

bool read_bytes(void *handle, int32_t ptr, void *buffer, std::size_t size);

bool write_bytes(void *handle, int32_t ptr, void *buffer, std::size_t size);

template <class T>
std::optional<T> read(void *handle, int32_t ptr) {
    T value{};
    if (read_bytes(handle, ptr, &value, sizeof(value))) {
        spdlog::trace("Read {}", value);
        return value;
    }
    return {};
};

template <class T>
bool write(void *handle, int32_t ptr, T value) {
    if (write_bytes(handle, ptr, &value, sizeof(value))) {
        spdlog::trace("Written {}", value);
        return true;
    }
    return false;
};

std::optional<std::string> read_string(void *handle, int32_t ptr, size_t size);

std::optional<int32_t> find_pointer(
    void *handle, int32_t ptr, const std::vector<int32_t> &offsets
);

bool read_bytes(
    void *handle,
    int32_t ptr,
    const std::vector<std::int32_t> &offsets,
    void *buffer,
    std::size_t size
);

template <class T>
std::optional<T> read(
    void *handle, int32_t ptr, const std::vector<std::int32_t> &offsets
) {
    auto ptr_ = find_pointer(handle, ptr, offsets);
    return ptr_ ? read<T>(handle, ptr) : std::nullopt;
};

std::optional<std::string> read_string(
    void *handle,
    int32_t ptr,
    const std::vector<std::int32_t> &offsets,
    std::size_t size
);
