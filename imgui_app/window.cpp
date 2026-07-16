#include "window.hpp"

#include <imgui_impl_win32.h>
#include <spdlog/spdlog.h>

#include <ranges>

#include "window_class.hpp"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam
);

// Win32 message handler
static LRESULT WINAPI
wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    imgui_app::Window::State* state;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lparam);
        state = reinterpret_cast<imgui_app::Window::State*>(cs->lpCreateParams);
        SetWindowLongPtr(
            hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state)
        );
    } else {
        state = reinterpret_cast<imgui_app::Window::State*>(
            GetWindowLongPtr(hwnd, GWLP_USERDATA)
        );
    }
    if (state && state->imgui_context) {
        ImGui::SetCurrentContext(state->imgui_context);
        // Must run before any on_message hook: it needs to observe every raw
        // input message to keep ImGui's IO state correct, even on messages
        // where it returns 0.
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) return 1;
        for (auto& callback : state->on_message | std::views::reverse) {
            if (auto result = callback(msg, wparam, lparam)) return *result;
        }
    }
    return ::DefWindowProcW(hwnd, msg, wparam, lparam);
}

imgui_app::WindowClassPtr imgui_app::create_window_class(
    std::wstring_view class_name, UINT style, HBRUSH background_brush
) {
    return imgui_app::detail::create_window_class(
        class_name, style, wnd_proc, background_brush
    );
}

void imgui_app::WindowDeleter::operator()(Window* window) const {
    if (window) {
        spdlog::debug(L"Destroying window {}...", window->title);
        if (window->handle) ::DestroyWindow(window->handle);
        delete window;
    }
}

imgui_app::WindowPtr imgui_app::create_window(
    std::shared_ptr<WindowClass> window_class,
    std::wstring_view title,
    DWORD dw_style,
    DWORD dw_ex_style,
    float character_width,
    float character_height,
    float font_size,
    std::optional<POINT> pos
) {
    spdlog::debug(L"Creating window {}...", title);
    if (!window_class) return nullptr;
    float dpiscale = ImGui_ImplWin32_GetDpiScaleForMonitor(
        ::MonitorFromPoint(pos.value_or(POINT{0, 0}), MONITOR_DEFAULTTOPRIMARY)
    );
    auto window = WindowPtr{new Window{}};
    window->window_class = window_class;
    window->title = title;
    window->state = std::make_unique<Window::State>();
    auto pos_cw = pos.value_or(POINT{CW_USEDEFAULT, CW_USEDEFAULT});
    window->handle = ::CreateWindowExW(
        dw_ex_style,
        window_class->cls.lpszClassName,
        window->title.c_str(),
        dw_style,
        pos_cw.x,
        pos_cw.y,
        character_width * font_size * dpiscale,
        character_height * font_size * dpiscale,
        nullptr,
        nullptr,
        window_class->cls.hInstance,
        window->state.get()
    );
    if (!window->handle) {
        spdlog::critical(L"Failed to create window {}", title);
        return nullptr;
    }
    if (dw_ex_style & WS_EX_LAYERED) {
        if (!::SetLayeredWindowAttributes(
                window->handle, RGB(0, 0, 0), 0, LWA_COLORKEY
            )) {
            spdlog::error("Failed to set layered window attributes");
        }
    }
    return window;
}
