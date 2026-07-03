#pragma once

#include <windows.h>

#include <memory>

struct Window {
    WNDCLASSEXW cls = {};
    ATOM atom = 0;
    HWND handle = nullptr;
};

struct WindowDeleter {
    void operator()(Window* window) const;
};

std::unique_ptr<Window, WindowDeleter> CreateWindowWin32(
    WNDPROC wnd_proc,
    const wchar_t* title,
    DWORD dw_style,
    DWORD dw_ex_style,
    int logical_width,
    int logical_height
);
