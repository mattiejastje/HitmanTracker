#pragma once

#include <windows.h>

#include <memory>

struct HandleDeleter {
    void operator()(void* process_handle) const;
};

using HandlePtr = std::unique_ptr<void, HandleDeleter>;

HandlePtr open_process_handle(DWORD process_id);
HandlePtr open_snapshot_handle();
