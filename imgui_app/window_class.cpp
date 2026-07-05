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
    }
}

std::shared_ptr<imgui_app::WindowClass> imgui_app::CreateWindowClass(
    WNDPROC wnd_proc, std::wstring_view class_name
) {
    spdlog::debug(L"Registering window class {}...", class_name);
    auto window_class
        = std::shared_ptr<WindowClass>(new WindowClass{}, WindowClassDeleter{});
    window_class->name = class_name;
    window_class->cls = WNDCLASSEXW{
        sizeof(WNDCLASSEXW),
        CS_CLASSDC,
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
