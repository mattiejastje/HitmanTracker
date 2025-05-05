#pragma once

#include <windows.h>

#include <memory>

struct ProcessHandleDeleter {
    void operator()(void* process_handle) const;
};

using HandlePtr = std::unique_ptr<void, ProcessHandleDeleter>;

HandlePtr open_process_handle(DWORD process_id);
HandlePtr open_snapshot_handle();
