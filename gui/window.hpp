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
    WNDPROC WndProc, float font_size, bool topmost
);
