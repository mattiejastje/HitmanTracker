#pragma once

#include <imgui.h>
#include <windows.h>

#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <unordered_set>

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

// forward declare for DrawResult
struct AppWindow;

struct DrawResult {
    std::unordered_set<AppWindow*> pending_rescale;
};

using DrawFunc = std::function<DrawResult(AppWindow&, float)>;

struct AppWindow {
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

void run(std::span<AppWindow*> app_windows);

}  // namespace imgui_app
