#include "process_handle.hpp"

#include "logger.hpp"

void ProcessHandleDeleter::operator()(void* process_handle) const {
    if (process_handle) {
        CloseHandle(process_handle);
        spdlog::debug("Process handle {} closed", process_handle);
    }
};

ProcessHandlePtr open_process_handle(DWORD process_id) {
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
    return ProcessHandlePtr{process_handle};
}
