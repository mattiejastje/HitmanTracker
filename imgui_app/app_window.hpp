#pragma once

#include <imgui.h>
#include <windows.h>

#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <unordered_set>
#include <variant>

#include "deviced3d.hpp"
#include "fonts.hpp"
#include "ui.hpp"

namespace imgui_app {

struct AppWindowSpec {
    std::wstring title;
    DWORD style;
    DWORD ex_style;
    float character_width;
    float character_height;
    std::optional<POINT> pos;
};

using TickFunc = std::function<void(float)>;

// forward declare for DrawResult
struct AppWindow;

using AppWindowSideEffect = std::function<void()>;

using DrawFunc = std::function<AppWindowSideEffect(AppWindow&, float)>;

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
    std::span<const FontSpec> fonts,
    DrawFunc draw
);

void run(TickFunc tick, std::span<AppWindow*> app_windows);

}  // namespace imgui_app
