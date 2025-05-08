#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

bool read_bytes(void *handle, int32_t ptr, void *buffer, std::size_t size);

bool write_bytes(void *handle, int32_t ptr, void *buffer, std::size_t size);

template <class T>
std::optional<T> read(void *handle, int32_t ptr) {
    T value{};
    return read_bytes(handle, ptr, &value, sizeof(value))
               ? std::make_optional(value)
               : std::nullopt;
};

template <class T>
bool write(void *handle, int32_t ptr, T value) {
    return write_bytes(handle, ptr, &value, sizeof(value));
};

std::optional<std::string> read_string(void *handle, int32_t ptr, size_t size);

std::optional<int32_t> find_pointer(
    void *handle, int32_t ptr, const std::vector<int32_t> &offsets
);

bool read_bytes(
    void *handle,
    int32_t ptr,
    const std::vector<int32_t> &offsets,
    void *buffer,
    std::size_t size
);

template <class T>
std::optional<T> read(
    void *handle, int32_t ptr, const std::vector<int32_t> &offsets
) {
    auto ptr_ = find_pointer(handle, ptr, offsets);
    return ptr_ ? read<T>(handle, ptr) : std::nullopt;
};

std::optional<std::string> read_string(
    void *handle,
    int32_t ptr,
    const std::vector<int32_t> &offsets,
    std::size_t size
);
