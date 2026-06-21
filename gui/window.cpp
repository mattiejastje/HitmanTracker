#include "window.hpp"

#include <imgui_impl_win32.h>

#include "../logging.hpp"

void WindowDeleter::operator()(Window* window) const {
    if (window) {
        logging::debug("Destroying application window...");
        if (window->handle) ::DestroyWindow(window->handle);
        if (window->atom != 0)
            ::UnregisterClassW(
                window->cls.lpszClassName, window->cls.hInstance
            );
    }
}

std::unique_ptr<Window, WindowDeleter> CreateWindowWin32(
    WNDPROC WndProc, float font_size, bool topmost
) {
    logging::debug("Creating application window...");
    auto window = std::unique_ptr<Window, WindowDeleter>(new Window{
        .cls
        = {sizeof(WNDCLASSEXW),
           CS_CLASSDC,
           WndProc,
           0L,
           0L,
           GetModuleHandle(nullptr),
           nullptr,
           nullptr,
           nullptr,
           nullptr,
           L"Hitman Tracker",
           nullptr}
    });
    window->atom = ::RegisterClassExW(&window->cls);
    if (window->atom == 0) return nullptr;
    // need to create the window before we know the dpi...
    // so first create 10x10 window then resize
    window->handle = ::CreateWindowExW(
        topmost ? WS_EX_TOPMOST : 0,
        window->cls.lpszClassName,
        L"Hitman Tracker",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        10,
        10,
        nullptr,
        nullptr,
        window->cls.hInstance,
        nullptr
    );
    float dpiscale = ImGui_ImplWin32_GetDpiScaleForHwnd(window->handle);
    ::SetWindowPos(
        window->handle,
        NULL,
        0,
        0,
        15 * font_size * dpiscale,
        30 * font_size * dpiscale,
        SWP_NOZORDER | SWP_NOMOVE
    );
    return window->handle ? std::move(window) : nullptr;
}
