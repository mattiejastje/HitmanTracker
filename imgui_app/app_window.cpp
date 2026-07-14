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
        spec.is_htclient_mapped_to_htcaption,
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
    // Keep the D3D backbuffer in sync with the window's client size.
    app_window_ptr->window->state->on_size.push_back(
        [app_window_ptr](UINT width, UINT height) {
            spdlog::debug("Handling window resize");
            auto& device = *app_window_ptr->device;
            device.state.present_parameters.BackBufferWidth = width;
            device.state.present_parameters.BackBufferHeight = height;
            ImGui::SetCurrentContext(app_window_ptr->ui->imgui_context);
            // can ignore if reset failed, will retry next frame
            std::ignore = imgui_app::ResetDevice(device);
        }
    );
    // Move to the OS-suggested rect and rescale fonts/UI for the new DPI.
    app_window_ptr->window->state->on_dpi_changed.push_back(
        [app_window_ptr](float dpiscale, const RECT& rect) {
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
        }
    );
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
        AppWindowActions all_actions{};
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
            auto actions = aw->draw(*aw, dt);
            ImGui::EndFrame();
            imgui_app::RenderAndPresent(device);
            all_actions.insert(
                all_actions.end(), actions.begin(), actions.end()
            );
        }
        for (auto& action : all_actions) {
            ImGui::SetCurrentContext(action.app_window->ui->imgui_context);
            std::visit(
                overloaded{
                    [&action](AppWindowAction::UpdateUIScaling args) {
                        if (!UpdateUIScaling(
                                *action.app_window->ui,
                                args.dpiscale.value_or(
                                    ImGui_ImplWin32_GetDpiScaleForHwnd(
                                        action.app_window->window->handle
                                    )
                                )
                            ))
                            spdlog::error("Failed to update UI scaling");
                    },
                    [&action](AppWindowAction::SetWinPos args) {
                        if (!::SetWindowPos(
                                action.app_window->window->handle,
                                args.hwnd_insert_after,
                                args.x,
                                args.y,
                                args.cx,
                                args.cy,
                                args.flags
                            ))
                            spdlog::error("Failed to set window position");
                    },
                    [&action](AppWindowAction::SetWinLongPtr args) {
                        auto value = GetWindowLongPtr(
                            action.app_window->window->handle, args.index
                        );
                        SetWindowLongPtr(
                            action.app_window->window->handle,
                            args.index,
                            args.value
                        );
                        spdlog::debug(
                            "Window long ptr {:#x} changed from {:#x} to {:#x}",
                            args.index,
                            value,
                            args.value
                        );
                    },
                    [&action](AppWindowAction::SetTransparentColorKey args) {
                        if (!::SetLayeredWindowAttributes(
                                action.app_window->window->handle,
                                RGB(0, 0, 0),
                                0,
                                LWA_COLORKEY
                            )) {
                            spdlog::error(
                                "Failed to set layered window attributes"
                            );
                        }
                    },
                },
                action.action
            );
        }
    }
    spdlog::info("Stopping main loop");
}