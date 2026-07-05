#pragma once

#include <windows.h>

#include <memory>
#include <optional>

#include "window_class.hpp"

namespace imgui_app {

struct Window {
    std::wstring title;
    HWND handle = nullptr;
};

struct WindowDeleter {
    void operator()(Window* window) const;
};

// windows.h defines CreateWindow as a macro...
[[nodiscard]] std::unique_ptr<Window, WindowDeleter> CreateAppWindow(
    std::shared_ptr<WindowClass> window_class,
    std::wstring_view title,
    DWORD dw_style,
    DWORD dw_ex_style,
    int logical_width,
    int logical_height,
    std::optional<POINT> pos = std::nullopt
);

}  // namespace imgui_app