#include "app_window.hpp"

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <spdlog/spdlog.h>

void imgui_app::AppWindowDeleter::operator()(AppWindow* app_window) const {
    spdlog::debug("Releasing AppWindow...");
    if (app_window) {
        // imgui_context pointer must be deleted first
        // otherwise WndProc might use dangling pointer
        app_window->window->state->imgui_context = nullptr;
        delete app_window;
    }
}

imgui_app::AppWindowPtr imgui_app::create_app_window(
    std::shared_ptr<WindowClass> window_class,
    const AppWindowSpec& spec,
    float font_size,
    ImVec4 bg_color,
    std::span<const FontSpec> font_specs,
    DrawFunc draw
) {
    if (!window_class) return nullptr;
    auto window = imgui_app::create_window(
        window_class,
        L"Hitman Tracker",
        spec.style,
        spec.ex_style,
        spec.character_width * font_size,
        spec.character_height * font_size
    );
    if (!window) return nullptr;
    auto dev = imgui_app::CreateDeviceD3D(window->handle);
    if (!dev) return nullptr;
    spdlog::debug("Showing window...");
    ::ShowWindow(window->handle, SW_SHOWDEFAULT);
    ::UpdateWindow(window->handle);
    auto ui = imgui_app::CreateUI(*window, *dev, bg_color, font_specs);
    if (!ui) return nullptr;
    window->state->imgui_context = ui->imgui_context;
    return AppWindowPtr{new AppWindow{
        window_class, std::move(window), std::move(dev), std::move(ui), draw
    }};
}

void imgui_app::run(std::span<AppWindowPtr> app_windows) {
    spdlog::debug("Starting main loop...");
    auto last_now = std::chrono::steady_clock::now();
    bool quit = false;
    while (!quit) {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT) quit = true;
        }
        if (quit) break;
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - last_now).count();
        last_now = now;
        for (auto& aw : app_windows) {
            auto& device = *aw->device;
            auto& ui = *aw->ui;
            auto& window = *aw->window;
            auto& state = *window.state;
            // skip render if device still lost
            if (imgui_app::HandleDeviceLost(device)) continue;
            spdlog::trace("New frame...");
            ImGui::SetCurrentContext(ui.imgui_context);
            if (state.resized) {
                spdlog::debug("Handling window resize");
                device.state.present_parameters.BackBufferWidth
                    = state.resize_width;
                device.state.present_parameters.BackBufferHeight
                    = state.resize_height;
                imgui_app::ResetDevice(device);
                state.resized = false;
            }
            if (state.dpi_changed) {
                spdlog::debug("Handling dpi change");
                const auto& dpi_rect = state.dpi_rect;
                ::SetWindowPos(
                    window.handle,
                    NULL,
                    dpi_rect.left,
                    dpi_rect.top,
                    dpi_rect.right - dpi_rect.left,
                    dpi_rect.bottom - dpi_rect.top,
                    SWP_NOZORDER
                );
                float dpiscale
                    = (float)state.dpi / (float)USER_DEFAULT_SCREEN_DPI;
                UpdateUIScaling(ui, dpiscale);
                state.dpi_changed = false;
            }
            if (aw->pending_rescale) {
                float dpiscale = ImGui_ImplWin32_GetDpiScaleForHwnd(window.handle);
                UpdateUIScaling(ui, dpiscale);
            }
            ImGui_ImplDX9_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            aw->draw(window.handle, ui, aw->pending_rescale, dt);
            ImGui::EndFrame();
            imgui_app::RenderAndPresent(device);
        }
    }
    spdlog::info("Stopping main loop");
}