#include "window.hpp"

#include <imgui_impl_win32.h>
#include <spdlog/spdlog.h>

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
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
            return true;
        switch (msg) {
            case WM_SIZE:
                if (wparam == SIZE_MINIMIZED) return 0;
                state->resize_width = (UINT)LOWORD(lparam);
                state->resize_height = (UINT)HIWORD(lparam);
                state->resized = true;
                return 0;
            case WM_DPICHANGED:
                assert(LOWORD(wparam) == HIWORD(wparam));
                CopyMemory(&state->dpi_rect, (RECT*)lparam, sizeof(RECT));
                state->dpi = LOWORD(wparam);
                state->dpi_changed = true;
                return 0;
            case WM_SYSCOMMAND:
                // Disable ALT application menu
                if ((wparam & 0xfff0) == SC_KEYMENU) return 0;
                break;
            case WM_DESTROY:
                ::PostQuitMessage(0);
                return 0;
        }
    }
    return ::DefWindowProcW(hwnd, msg, wparam, lparam);
}

imgui_app::WindowClassPtr imgui_app::create_window_class() {
    return imgui_app::detail::create_window_class(
        L"ImGuiAppWindowClass", CS_OWNDC, wnd_proc
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
        logical_width * dpiscale,
        logical_height * dpiscale,
        nullptr,
        nullptr,
        window_class->cls.hInstance,
        window->state.get()
    );
    if (!window->handle) {
        spdlog::critical(L"Failed to create window {}", title);
        return nullptr;
    }
    return window;
}
