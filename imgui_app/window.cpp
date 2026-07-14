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
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) return 1;
        switch (msg) {
            case WM_SIZE:
                if (wparam == SIZE_MINIMIZED) return 0;
                for (auto& callback : state->on_size)
                    callback((UINT)LOWORD(lparam), (UINT)HIWORD(lparam));
                return 0;
            case WM_DPICHANGED: {
                assert(LOWORD(wparam) == HIWORD(wparam));
                float dpiscale = static_cast<float>(LOWORD(wparam))
                                 / USER_DEFAULT_SCREEN_DPI;
                const RECT& rect = *reinterpret_cast<RECT*>(lparam);
                for (auto& callback : state->on_dpi_changed)
                    callback(dpiscale, rect);
                return 0;
            }
            case WM_EXITSIZEMOVE: {
                RECT rect;
                if (::GetWindowRect(hwnd, &rect))
                    for (auto& callback : state->on_exit_size_move)
                        callback(rect);
                return 0;
            }
            case WM_SYSCOMMAND:
                // Disable ALT application menu
                if ((wparam & 0xfff0) == SC_KEYMENU) return 0;
                break;
            case WM_ERASEBKGND: {
                HDC hdc = (HDC)wparam;
                RECT rc;
                GetClientRect(hwnd, &rc);
                HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
                FillRect(hdc, &rc, hBrush);
                DeleteObject(hBrush);
                return TRUE;
            }
            case WM_SETCURSOR:
                if (state->is_htclient_mapped_to_htcaption
                    && LOWORD(lparam) == HTCAPTION) {
                    SetCursor(LoadCursor(NULL, IDC_SIZEALL));
                    return TRUE;
                }
                break;
            case WM_NCHITTEST:
                if (state->is_htclient_mapped_to_htcaption) {
                    POINT pt;
                    GetCursorPos(&pt);
                    ScreenToClient(hwnd, &pt);
                    RECT rc;
                    GetClientRect(hwnd, &rc);
                    const int border = 10;
                    if (pt.x < border && pt.y < border) return HTTOPLEFT;
                    if (pt.x > rc.right - border && pt.y < border)
                        return HTTOPRIGHT;
                    if (pt.x < border && pt.y > rc.bottom - border)
                        return HTBOTTOMLEFT;
                    if (pt.x > rc.right - border && pt.y > rc.bottom - border)
                        return HTBOTTOMRIGHT;
                    if (pt.x < border) return HTLEFT;
                    if (pt.x > rc.right - border) return HTRIGHT;
                    if (pt.y < border) return HTTOP;
                    if (pt.y > rc.bottom - border) return HTBOTTOM;
                    auto hit = ::DefWindowProcW(hwnd, msg, wparam, lparam);
                    return hit == HTCLIENT ? HTCAPTION : hit;
                }
                break;
            case WM_DESTROY:
                ::PostQuitMessage(0);
                return 0;
        }
    }
    return ::DefWindowProcW(hwnd, msg, wparam, lparam);
}

imgui_app::WindowClassPtr imgui_app::create_window_class(
    std::wstring_view class_name, UINT style
) {
    return imgui_app::detail::create_window_class(class_name, style, wnd_proc);
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
    bool is_htclient_mapped_to_htcaption,
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
    window->state->is_htclient_mapped_to_htcaption
        = is_htclient_mapped_to_htcaption;
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
