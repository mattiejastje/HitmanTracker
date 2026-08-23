#pragma once

#include <spdlog/spdlog.h>

#include <cassert>
#include <concepts>
#include <cstdint>
#include <mempeep/read.hpp>
#include <mempeep/tracers/log_tracer.hpp>
#include <optional>
#include <string>
#include <vector>

bool read_bytes(void* handle, intptr_t ptr, void* buffer, intptr_t size);

bool write_bytes(void* handle, intptr_t ptr, void* buffer, intptr_t size);

intptr_t take_bytes_read();

intptr_t take_bytes_written();

template <class T>
std::optional<T> read(void* handle, intptr_t ptr) {
    T value{};
    return read_bytes(handle, ptr, &value, sizeof(value))
               ? std::make_optional(value)
               : std::nullopt;
};

template <class T>
bool write(void* handle, intptr_t ptr, T value) {
    return write_bytes(handle, ptr, &value, sizeof(value));
};

std::optional<std::string> read_string(
    void* handle, intptr_t ptr, intptr_t size
);

template <class P>
    requires std::unsigned_integral<P>
std::optional<intptr_t> find_pointer(
    void* handle, intptr_t ptr, const std::vector<intptr_t>& offsets
) {
    constexpr P ptr_max = std::numeric_limits<P>::max();
    for (auto offset : offsets) {
        spdlog::trace("Finding [{:#x}]+{:#x}", ptr, offset);
        auto next_ptr = read<P>(handle, ptr);
        if (!next_ptr) return {};
        P p = next_ptr.value();
        if (p == 0) {
            spdlog::trace("Pointer is null");
            return {};
        }
        if (offset >= 0) {
            if (!std::in_range<P>(offset)) {
                spdlog::error("Offset too large for pointer type");
                return {};
            }
            P uoffset = static_cast<P>(offset);
            if (p > ptr_max - uoffset) {
                spdlog::trace("Pointer offset overflow");
                return {};
            }
            p += uoffset;
        } else {
            auto uoffset1
                = static_cast<uintptr_t>(0) - static_cast<uintptr_t>(offset);
            if (!std::in_range<P>(uoffset1)) {
                spdlog::error("Offset too large for pointer type");
                return {};
            }
            P uoffset2 = static_cast<P>(uoffset1);
            if (p <= uoffset2) {
                spdlog::trace("Pointer offset underflow");
                return {};
            }
            p -= uoffset2;
        }
        spdlog::trace("Found {:#x}", p);
        if (!std::in_range<intptr_t>(p)) {
            spdlog::error("Pointer does not fit intptr_t");
            return {};
        }
        ptr = static_cast<intptr_t>(p);
    }
    return ptr;
};

template <class P>
bool read_bytes(
    void* handle,
    intptr_t ptr,
    const std::vector<intptr_t>& offsets,
    void* buffer,
    intptr_t size
) {
    auto ptr_ = find_pointer<P>(handle, ptr, offsets);
    return ptr_ ? read_bytes(handle, ptr_.value(), buffer, size) : false;
};

template <class P, class T>
std::optional<T> read(
    void* handle, intptr_t ptr, const std::vector<intptr_t>& offsets
) {
    auto ptr_ = find_pointer<P>(handle, ptr, offsets);
    return ptr_ ? read<T>(handle, ptr_.value()) : std::nullopt;
};

template <class P>
std::optional<std::string> read_string(
    void* handle,
    intptr_t ptr,
    const std::vector<intptr_t>& offsets,
    P ptr_max,
    intptr_t size
) {
    auto ptr_ = find_pointer<P>(handle, ptr, offsets);
    return ptr_ ? read_string(handle, ptr_.value(), size) : std::nullopt;
};

template <typename Address>
    requires mempeep::IsAddress<Address>
struct MemoryReader {
    using address_type = Address;
    void* handle;

    bool operator()(Address address, std::size_t size, void* buffer) const {
        if (buffer == nullptr) return false;
        if (size == 0) return false;
        return read_bytes(handle, static_cast<intptr_t>(address), buffer, size);
    }
};

struct MempeepOnLogEntry {
    template <typename T>
    void operator()(const mempeep::LogEntry<T>& entry) const {
        if constexpr (std::same_as<T, mempeep::Error>) {
            spdlog::warn("{}", entry);
        } else {
            spdlog::trace("{}", entry);
        }
    }
};

template <
    mempeep::IsDescriptor Desc,
    mempeep::IsMemoryReader Reader,
    mempeep::IsTracer Tracer>
bool read_at_address(
    mempeep::address_t<Reader> address,
    Reader& reader,
    Tracer& tracer,
    mempeep::native_type_t<Desc>& out
) {
    return mempeep::read(
        mempeep::RemoteValue<Desc, mempeep::address_t<Reader>>{address},
        reader,
        tracer,
        out
    );
}
