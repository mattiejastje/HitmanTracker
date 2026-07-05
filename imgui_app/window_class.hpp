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

[[nodiscard]] std::shared_ptr<WindowClass> CreateWindowClass(
    WNDPROC wnd_proc, std::wstring_view class_name
);

}  // namespace imgui_app