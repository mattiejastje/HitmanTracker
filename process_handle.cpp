#include "process_handle.hpp"

#include <tlhelp32.h>

#include "logger.hpp"

void ProcessHandleDeleter::operator()(void* process_handle) const {
    if (process_handle) {
        CloseHandle(process_handle);
        spdlog::debug("Process handle {} closed", process_handle);
    }
};

HandlePtr open_process_handle(DWORD process_id) {
    auto process_handle = OpenProcess(PROCESS_ALL_ACCESS, 0, process_id);
    if (process_handle) {
        spdlog::debug(
            "Process handle {} opened for process id {}",
            process_handle,
            process_id
        );
    } else {
        spdlog::error("Cannot open handle for process id {}", process_id);
    }
    return HandlePtr{process_handle};
}

HandlePtr open_snapshot_handle() {
    auto snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot_handle == INVALID_HANDLE_VALUE) snapshot_handle = nullptr;
    if (snapshot_handle) {
        spdlog::debug(
            "Handle {} opened for snapshot of all processes", snapshot_handle
        );
    } else {
        spdlog::error("Cannot open handle for snapshot of all processes");
    }
    return HandlePtr{snapshot_handle};
}
