#pragma once

#include <windows.h>

#include <memory>
#include <string>

namespace imgui_app {

struct WindowClass {
    std::wstring name;  // ensure ownership, cls only stores pointer
    WNDCLASSEXW cls = {};
    ATOM atom = 0;
};

struct WindowClassDeleter {
    void operator()(WindowClass* window_class) const;
};

using WindowClassPtr = std::unique_ptr<WindowClass, WindowClassDeleter>;

}  // namespace imgui_app

namespace imgui_app::detail {

[[nodiscard]] WindowClassPtr create_window_class(
    std::wstring_view class_name,
    UINT style,
    WNDPROC wnd_proc,
    HBRUSH background_brush
);

}
