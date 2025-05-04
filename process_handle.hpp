#pragma once

#include <windows.h>

#include <memory>

struct ProcessHandleDeleter {
    void operator()(void* process_handle) const;
};

using ProcessHandlePtr = std::unique_ptr<void, ProcessHandleDeleter>;

ProcessHandlePtr open_process_handle(DWORD process_id);
