#pragma once

#include <imgui.h>
#include <windows.h>

#include <memory>
#include <optional>

#include "window_class.hpp"

namespace imgui_app {

[[nodiscard]] WindowClassPtr create_window_class();

struct Window {
    struct State {
        bool resized = false;
        UINT resize_width = 0;
        UINT resize_height = 0;
        bool dpi_changed = false;
        RECT dpi_rect{};
        UINT dpi = 0;
        ImGuiContext* imgui_context = nullptr;
    };

    std::shared_ptr<WindowClass> window_class;
    std::wstring title;
    HWND handle = nullptr;
    // unique_ptr ensures stable pointer
    // even if Window object is moved
    std::unique_ptr<State> state;
};

struct WindowDeleter {
    void operator()(Window* window) const;
};

using WindowPtr = std::unique_ptr<Window, WindowDeleter>;

// windows.h defines CreateWindow as a macro...
[[nodiscard]] WindowPtr create_window(
    std::shared_ptr<WindowClass> window_class,
    std::wstring_view title,
    DWORD dw_style,
    DWORD dw_ex_style,
    int logical_width,
    int logical_height,
    std::optional<POINT> pos = std::nullopt
);

}  // namespace imgui_app