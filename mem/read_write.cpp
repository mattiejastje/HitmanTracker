#include "read_write.hpp"

#include <windows.h>

static intptr_t g_bytes_read = 0;
static intptr_t g_bytes_written = 0;

static bool read_process_memory(
    void* handle, void* ptr, void* buffer, intptr_t size
) {
    SIZE_T bytes_read = 0;
    return ReadProcessMemory(handle, ptr, buffer, size, &bytes_read);
}

static bool write_process_memory(
    void* handle, void* ptr, void* buffer, intptr_t size
) {
    SIZE_T bytes_written = 0;
    return WriteProcessMemory(handle, ptr, buffer, size, &bytes_written);
}

bool read_bytes(void* handle, intptr_t ptr, void* buffer, intptr_t size) {
    if (handle && ptr) {
        if (read_process_memory(
                handle, reinterpret_cast<void*>(ptr), buffer, size
            )) {
            g_bytes_read += size;
            spdlog::trace("Read {} bytes at {:#x}", size, ptr);
            return true;
        }
    }
    spdlog::trace("Failed to read {} bytes at {:#x}", size, ptr);
    return false;
}

bool write_bytes(void* handle, intptr_t ptr, void* buffer, intptr_t size) {
    if (handle && ptr) {
        if (write_process_memory(
                handle, reinterpret_cast<void*>(ptr), buffer, size
            )) {
            g_bytes_written += size;
            spdlog::trace("Written {} bytes at {:#x}", size, ptr);
            return true;
        }
    }
    spdlog::trace("Failed to write {} bytes at {:#x}", size, ptr);
    return false;
}

intptr_t take_bytes_read() { return std::exchange(g_bytes_read, 0); }

intptr_t take_bytes_written() { return std::exchange(g_bytes_written, 0); }

std::optional<std::string> read_string(
    void* handle, intptr_t ptr, intptr_t size
) {
    auto value = std::make_unique<char[]>(size);
    if (read_bytes(handle, ptr, value.get(), size)) {
        auto result = std::string(value.get(), strnlen(value.get(), size));
        spdlog::trace("Read string {}", result);
        return result;
    }
    return {};
}

mempeep::LogTracer<MempeepOnLogEntry> make_mempeep_log_tracer() {
    return mempeep::LogTracer{MempeepOnLogEntry{}, mempeep::LogLevel::ERRORS};
}
