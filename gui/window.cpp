#include "window.hpp"

#include <imgui_impl_win32.h>

#include <spdlog/spdlog.h>

void WindowDeleter::operator()(Window* window) const {
    if (window) {
        spdlog::debug("Destroying application window...");
        if (window->handle) ::DestroyWindow(window->handle);
        if (window->atom != 0)
            ::UnregisterClassW(
                window->cls.lpszClassName, window->cls.hInstance
            );
    }
}

std::unique_ptr<Window, WindowDeleter> CreateWindowWin32(
    WNDPROC wnd_proc,
    const wchar_t* title,
    DWORD dw_style,
    DWORD dw_ex_style,
    int logical_width,
    int logical_height
) {
    spdlog::debug("Creating application window...");
    auto window = std::unique_ptr<Window, WindowDeleter>(new Window{
        .cls
        = {sizeof(WNDCLASSEXW),
           CS_CLASSDC,
           wnd_proc,
           0L,
           0L,
           GetModuleHandle(nullptr),
           nullptr,
           nullptr,
           nullptr,
           nullptr,
           title,
           nullptr}
    });
    window->atom = ::RegisterClassExW(&window->cls);
    if (window->atom == 0) return nullptr;
    float dpiscale = ImGui_ImplWin32_GetDpiScaleForMonitor(
        ::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY)
    );
    window->handle = ::CreateWindowExW(
        dw_ex_style,
        window->cls.lpszClassName,
        title,
        dw_style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        logical_width * dpiscale,
        logical_height * dpiscale,
        nullptr,
        nullptr,
        window->cls.hInstance,
        nullptr
    );
    return window->handle ? std::move(window) : nullptr;
}
