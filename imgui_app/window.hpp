#pragma once

#include <imgui.h>
#include <windows.h>

#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include "window_class.hpp"

namespace imgui_app {

[[nodiscard]] WindowClassPtr create_window_class(
    std::wstring_view class_name, UINT style, HBRUSH background_brush
);

using ResizeCallback = std::function<void(UINT width, UINT height)>;

using DpiChangedCallback
    = std::function<void(float dpiscale, const RECT& suggested_rect)>;

using ExitSizeMoveCallback = std::function<void(const RECT& rect)>;

struct Window {
    struct State {
        ImGuiContext* imgui_context = nullptr;
        // makes client area behave like title bar
        // so window can be dragged around
        bool is_htclient_mapped_to_htcaption = false;
        // Consumers append here (typically right after the window is
        // created) to react to window events. wnd_proc invokes these
        // directly, synchronously, whenever the corresponding message
        // arrives. Changes that originate *inside* a draw()
        // callback (e.g. a settings checkbox triggering a resize) must
        // NOT call into these directly. Use AppWindowAction, returned
        // from draw() and applied once every window's frame has ended,
        // instead.
        std::vector<ResizeCallback> on_size;
        std::vector<DpiChangedCallback> on_dpi_changed;
        std::vector<ExitSizeMoveCallback> on_exit_size_move;
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
    float character_width,
    float character_height,
    float font_size,
    bool is_htclient_mapped_to_htcaption,
    std::optional<POINT> pos = std::nullopt
);

}  // namespace imgui_app