#pragma once

#include <imgui.h>
#include <windows.h>

#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>

#include "deviced3d.hpp"
#include "fonts.hpp"
#include "ui.hpp"

namespace imgui_app {

struct AppWindowSpec {
    std::wstring title;
    DWORD style;
    DWORD ex_style;
    int character_width, character_height;
    std::optional<POINT> pos;
};

struct DrawResult {
    bool pending_rescale;
};

using DrawFunc = std::function<DrawResult(HWND, UI&, float)>;

struct AppWindow {
    std::shared_ptr<WindowClass> window_class;
    WindowPtr window;
    DeviceD3DPtr device;
    UIPtr ui;
    DrawFunc draw;
};

struct AppWindowDeleter {
    void operator()(AppWindow* app_window) const;
};

using AppWindowPtr = std::unique_ptr<AppWindow, AppWindowDeleter>;

AppWindowPtr create_app_window(
    std::shared_ptr<WindowClass> window_class,
    const AppWindowSpec& spec,
    float font_size,
    ImVec4 bg_color,
    std::span<const FontSpec> fonts,
    DrawFunc draw
);

void run(std::span<AppWindowPtr> app_windows);

}  // namespace imgui_app
