#include "window_class.hpp"

#include <spdlog/spdlog.h>

void imgui_app::WindowClassDeleter::operator()(
    WindowClass* window_class
) const {
    if (window_class) {
        spdlog::debug(L"Unregistering window class {}...", window_class->name);
        if (window_class->atom != 0)
            ::UnregisterClassW(
                window_class->cls.lpszClassName, window_class->cls.hInstance
            );
        delete window_class;
    }
}

imgui_app::WindowClassPtr imgui_app::detail::create_window_class(
    std::wstring_view class_name, UINT style, WNDPROC wnd_proc
) {
    spdlog::debug(L"Registering window class {}...", class_name);
    auto window_class = WindowClassPtr{new WindowClass{}};
    window_class->name = class_name;
    window_class->cls = WNDCLASSEXW{
        sizeof(WNDCLASSEXW),
        style,
        wnd_proc,
        0L,
        0L,
        GetModuleHandle(nullptr),
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        window_class->name.c_str(),
        nullptr
    };
    window_class->atom = ::RegisterClassExW(&window_class->cls);
    if (window_class->atom == 0) {
        spdlog::critical(L"Failed to register window class {}", class_name);
        return nullptr;
    }
    return window_class;
}
