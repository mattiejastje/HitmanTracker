#include "app_window.hpp"

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <spdlog/spdlog.h>

#include "../overloaded.hpp"

void imgui_app::AppWindowDeleter::operator()(AppWindow* app_window) const {
    spdlog::debug("Releasing AppWindow...");
    if (app_window) {
        // imgui_context pointer must be deleted first
        // otherwise WndProc might use dangling pointer
        app_window->window->state->imgui_context = nullptr;
        app_window->ui.reset();
        app_window->device.reset();
        app_window->window.reset();
        delete app_window;
    }
}

imgui_app::AppWindowPtr imgui_app::create_app_window(
    std::shared_ptr<WindowClass> window_class,
    const AppWindowSpec& spec,
    float font_size,
    std::span<const FontSpec> font_specs,
    DrawFunc draw
) {
    if (!window_class) return nullptr;
    auto window = imgui_app::create_window(
        window_class,
        spec.title,
        spec.style,
        spec.ex_style,
        spec.character_width,
        spec.character_height,
        font_size,
        spec.pos
    );
    if (!window) return nullptr;
    auto dev = imgui_app::CreateDeviceD3D(window->handle);
    if (!dev) return nullptr;
    spdlog::debug("Showing window...");
    ::ShowWindow(window->handle, SW_SHOWDEFAULT);
    ::UpdateWindow(window->handle);
    auto ui
        = imgui_app::CreateUI(*window, *dev, ImVec4{0, 0, 0, 1}, font_specs);
    if (!ui) return nullptr;
    window->state->imgui_context = ui->imgui_context;
    auto* app_window_ptr
        = new AppWindow{std::move(window), std::move(dev), std::move(ui), draw};
    AppWindowPtr app_window{app_window_ptr};
    OnMessage on_message = [app_window_ptr](
                               UINT msg, WPARAM wparam, LPARAM lparam
                           ) -> std::optional<LRESULT> {
        switch (msg) {
            case WM_SIZE: {
                // keep backbuffer in sync with window size
                if (wparam == SIZE_MINIMIZED) return 0;
                spdlog::debug("Handling window resize");
                auto& device = *app_window_ptr->device;
                device.state.present_parameters.BackBufferWidth
                    = (UINT)LOWORD(lparam);
                device.state.present_parameters.BackBufferHeight
                    = (UINT)HIWORD(lparam);
                ImGui::SetCurrentContext(app_window_ptr->ui->imgui_context);
                // can ignore if reset failed, will retry next frame
                std::ignore = imgui_app::ResetDevice(device);
                return 0;
            }
            case WM_DPICHANGED: {
                // move to suggested rect and rescale fonts/UI
                assert(LOWORD(wparam) == HIWORD(wparam));
                float dpiscale = static_cast<float>(LOWORD(wparam))
                                 / USER_DEFAULT_SCREEN_DPI;
                const RECT& rect = *reinterpret_cast<RECT*>(lparam);
                spdlog::debug("Handling dpi change");
                ::SetWindowPos(
                    app_window_ptr->window->handle,
                    nullptr,
                    rect.left,
                    rect.top,
                    rect.right - rect.left,
                    rect.bottom - rect.top,
                    SWP_NOZORDER
                );
                ImGui::SetCurrentContext(app_window_ptr->ui->imgui_context);
                imgui_app::UpdateUIScaling(*app_window_ptr->ui, dpiscale);
                return 0;
            }
            case WM_SYSCOMMAND:
                // disable ALT application menu
                if ((wparam & 0xfff0) == SC_KEYMENU) return 0;
                break;
            case WM_DESTROY:
                ::PostQuitMessage(0);
                return 0;
        };
        return std::nullopt;
    };
    app_window_ptr->window->state->on_message.push_back(on_message);
    return app_window;
}

void imgui_app::run(TickFunc tick, std::span<AppWindow*> app_windows) {
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
        tick(dt);
        std::vector<AppWindowSideEffect> side_effects{};
        for (auto& aw : app_windows) {
            auto& device = *aw->device;
            auto& ui = *aw->ui;
            auto& window = *aw->window;
            auto& state = *window.state;
            // skip render if device not ready
            if (!imgui_app::IsDeviceReady(device)) continue;
            spdlog::trace("New frame...");
            ImGui::SetCurrentContext(ui.imgui_context);
            ImGui_ImplDX9_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            auto& io = ImGui::GetIO();
            ImGui::SetNextWindowSize({io.DisplaySize.x, io.DisplaySize.y});
            ImGui::SetNextWindowPos({0, 0});
            auto side_effect = aw->draw(*aw, dt);
            if (side_effect) side_effects.push_back(side_effect);
            ImGui::EndFrame();
            imgui_app::RenderAndPresent(device);
        }
        for (auto& side_effect : side_effects) side_effect();
    }
    spdlog::info("Stopping main loop");
}