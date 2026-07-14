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
    std::wstring_view class_name,
    UINT style,
    WNDPROC wnd_proc,
    HBRUSH background_brush
) {
    spdlog::debug(L"Registering window class {}...", class_name);
    auto window_class = WindowClassPtr{new WindowClass{}};
    window_class->name = class_name;
    window_class->cls = WNDCLASSEXW{
        .cbSize = sizeof(WNDCLASSEXW),
        .style = style,
        .lpfnWndProc = wnd_proc,
        .cbClsExtra = 0L,
        .cbWndExtra = 0L,
        .hInstance = GetModuleHandle(nullptr),
        .hIcon = nullptr,
        .hCursor = nullptr,
        .hbrBackground = background_brush,
        .lpszMenuName = nullptr,
        .lpszClassName = window_class->name.c_str(),
        .hIconSm = nullptr
    };
    window_class->atom = ::RegisterClassExW(&window_class->cls);
    if (window_class->atom == 0) {
        spdlog::critical(L"Failed to register window class {}", class_name);
        return nullptr;
    }
    return window_class;
}
