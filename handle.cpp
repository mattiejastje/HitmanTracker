#include "handle.hpp"

#include <tlhelp32.h>

#include "logger.hpp"

void HandleDeleter::operator()(void* handle) const {
    if (handle) {
        CloseHandle(handle);
        spdlog::debug("Handle {} closed", handle);
    }
};

HandlePtr open_process_handle(DWORD process_id) {
    auto process_handle = OpenProcess(PROCESS_ALL_ACCESS, 0, process_id);
    if (process_handle) {
        spdlog::debug(
            "Handle {} opened for process id {:#x}", process_handle, process_id
        );
    } else {
        spdlog::error("Cannot open handle for process id {:#x}", process_id);
    }
    return HandlePtr{process_handle};
}

HandlePtr open_snapshot_handle(DWORD flags, DWORD process_id) {
    auto snapshot_handle = CreateToolhelp32Snapshot(flags, process_id);
    if (snapshot_handle == INVALID_HANDLE_VALUE) snapshot_handle = nullptr;
    if (snapshot_handle) {
        spdlog::debug(
            "Handle {} opened for snapshot", snapshot_handle
        );
    } else {
        spdlog::error("Cannot open handle for snapshot");
    }
    return HandlePtr{snapshot_handle};
}
