#pragma once

#include <cassert>
#include <concepts>
#include <cstdint>
#include <mempeep/detail/concepts/address.hpp>
#include <mempeep/tracers/log_tracer.hpp>
#include <optional>
#include <string>
#include <vector>

#include "../logging.hpp"

bool read_bytes(void *handle, intptr_t ptr, void *buffer, intptr_t size);

bool write_bytes(void *handle, intptr_t ptr, void *buffer, intptr_t size);

template <class T>
std::optional<T> read(void *handle, intptr_t ptr) {
    T value{};
    return read_bytes(handle, ptr, &value, sizeof(value))
               ? std::make_optional(value)
               : std::nullopt;
};

template <class T>
bool write(void *handle, intptr_t ptr, T value) {
    return write_bytes(handle, ptr, &value, sizeof(value));
};

std::optional<std::string> read_string(
    void *handle, intptr_t ptr, intptr_t size
);

template <class P>
std::optional<intptr_t> find_pointer(
    void *handle, intptr_t ptr, const std::vector<intptr_t> &offsets, P ptr_max
) {
    for (auto offset : offsets) {
        logging::trace("Finding [{:#x}]+{:#x}", ptr, offset);
        auto next_ptr = read<P>(handle, ptr);
        if (!next_ptr) return {};
        ptr = next_ptr.value();
        if (ptr <= 0) {
            logging::trace("Pointer zero or negative");
            return {};
        }
        if (offset >= 0 && ptr > (ptr_max - offset)) {
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
};

template <class P>
bool read_bytes(
    void *handle,
    intptr_t ptr,
    const std::vector<intptr_t> &offsets,
    P ptr_max,
    void *buffer,
    intptr_t size
) {
    auto ptr_ = find_pointer(handle, ptr, offsets, ptr_max);
    return ptr_ ? read_bytes(handle, ptr_.value(), buffer, size) : false;
};

template <class T, class P>
std::optional<T> read(
    void *handle, intptr_t ptr, const std::vector<intptr_t> &offsets, P ptr_max
) {
    auto ptr_ = find_pointer(handle, ptr, offsets, ptr_max);
    return ptr_ ? read<T>(handle, ptr_.value()) : std::nullopt;
};

template <class P>
std::optional<std::string> read_string(
    void *handle,
    intptr_t ptr,
    const std::vector<intptr_t> &offsets,
    P ptr_max,
    intptr_t size
) {
    auto ptr_ = find_pointer(handle, ptr, offsets, ptr_max);
    return ptr_ ? read_string(handle, ptr_.value(), size) : std::nullopt;
};

template <typename Address>
    requires mempeep::IsAddress<Address>
struct MemoryReader {
    using address_type = Address;
    void *handle;

    bool operator()(Address address, std::size_t size, void *buffer) const {
        if (buffer == nullptr) return false;
        if (size == 0) return false;
        return read_bytes(handle, static_cast<intptr_t>(address), buffer, size);
    }
};

struct MempeepOnLogEntry {
    template <typename T>
    void operator()(const mempeep::LogEntry<T> &entry) const {
        if constexpr (std::same_as<T, mempeep::Error>) {
            logging::warn("{}", entry);
        } else {
            logging::trace("{}", entry);
        }
    }
};
