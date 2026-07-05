#include "window.hpp"

#include <imgui_impl_win32.h>
#include <spdlog/spdlog.h>

#include "window_class.hpp"

void imgui_app::WindowDeleter::operator()(Window* window) const {
    if (window) {
        spdlog::debug(L"Destroying window {}...", window->title);
        if (window->handle) ::DestroyWindow(window->handle);
    }
}

std::unique_ptr<imgui_app::Window, imgui_app::WindowDeleter>
imgui_app::CreateAppWindow(
    std::shared_ptr<WindowClass> window_class,
    std::wstring_view title,
    DWORD dw_style,
    DWORD dw_ex_style,
    int logical_width,
    int logical_height,
    std::optional<POINT> pos
) {
    spdlog::debug(L"Creating window {}...", title);
    if (!window_class) return nullptr;
    float dpiscale = ImGui_ImplWin32_GetDpiScaleForMonitor(
        ::MonitorFromPoint(pos.value_or(POINT{0, 0}), MONITOR_DEFAULTTOPRIMARY)
    );
    auto window = std::unique_ptr<Window, WindowDeleter>(new Window{});
    window->title = title;
    auto pos_cw = pos.value_or(POINT{CW_USEDEFAULT, CW_USEDEFAULT});
    window->handle = ::CreateWindowExW(
        dw_ex_style,
        window_class->cls.lpszClassName,
        window->title.c_str(),
        dw_style,
        pos_cw.x,
        pos_cw.y,
        logical_width * dpiscale,
        logical_height * dpiscale,
        nullptr,
        nullptr,
        window_class->cls.hInstance,
        nullptr
    );
    if (!window->handle) {
        spdlog::critical(L"Failed to create window {}", title);
        return nullptr;
    }

    return window;
}
