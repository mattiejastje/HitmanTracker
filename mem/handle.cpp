#include "handle.hpp"

#include <spdlog/spdlog.h>
#include <tlhelp32.h>

void HandleDeleter::operator()(void* handle) const {
    if (handle) {
        CloseHandle(handle);
        spdlog::debug("Handle {} closed", handle);
    }
};

bool is_process_running(void* process_handle) {
    DWORD exit_code{};
    if (!GetExitCodeProcess(process_handle, &exit_code)) return false;
    return exit_code == STILL_ACTIVE;
}

HandlePtr open_process_handle(DWORD process_id) {
    auto process_handle = OpenProcess(
        PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION
            | PROCESS_QUERY_LIMITED_INFORMATION,
        0,
        process_id
    );
    if (process_handle) {
        spdlog::debug(
            "Handle {} opened for process id {:#x}", process_handle, process_id
        );
    } else {
        auto msg = GetLastError() == 0x5 ? "Access denied for" : "Cannot open";
        spdlog::error("{} process id {:#x}", msg, process_id);
    }
    return HandlePtr{process_handle};
}

HandlePtr open_snapshot_handle(DWORD flags, DWORD process_id) {
    auto snapshot_handle = CreateToolhelp32Snapshot(flags, process_id);
    if (snapshot_handle == INVALID_HANDLE_VALUE) snapshot_handle = nullptr;
    if (snapshot_handle) {
        spdlog::debug("Handle {} opened for snapshot", snapshot_handle);
    } else {
        spdlog::error("Cannot open handle for snapshot");
    }
    return HandlePtr{snapshot_handle};
}
