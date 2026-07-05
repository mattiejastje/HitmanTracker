#include "gui.hpp"

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <spdlog/spdlog.h>
#include <tchar.h>

#include <CLI/CLI.hpp>
#include <algorithm>
#include <filesystem>
#include <map>
#include <tuple>

#include "color.hpp"
#include "game.hpp"
#include "hitman_common/gui.hpp"
#include "imgui_app/deviced3d.hpp"
#include "imgui_app/text.hpp"
#include "imgui_app/ui.hpp"
#include "imgui_app/window.hpp"
#include "mem/handle.hpp"
#include "settings_gui.hpp"
#include "signal.hpp"
#include "timer.hpp"

// Data
static timer::PeriodicTimer timer_find_game{1.0};
static timer::PeriodicTimer timer_update_stats{0.1};
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
static RECT g_ChangeRect = {};
static UINT g_ChangeDpi = 0;
static std::optional<Game> game{};
static Signal frametime_signal{"fps", "frames per second"};
static Signal error_slow{"slow update failure rate", "%", 50.0f};
static Signal error_fast{"fast update failure rate", "%", 50.0f};
static Profiler profiler_slow{{"slow update time", "seconds"}};
static Profiler profiler_fast{{"fast update time", "seconds"}};
static SettingsChanged g_settings_changed{};

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
);

// Win32 message handler
static LRESULT WINAPI
WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
    switch (msg) {
        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED) return 0;
            g_ResizeWidth = (UINT)LOWORD(lParam);
            g_ResizeHeight = (UINT)HIWORD(lParam);
            return 0;
        case WM_SYSCOMMAND:
            // Disable ALT application menu
            if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
            break;
        case WM_DPICHANGED:
            assert(LOWORD(wParam) == HIWORD(wParam));
            CopyMemory(&g_ChangeRect, (RECT*)lParam, sizeof(RECT));
            g_ChangeDpi = LOWORD(wParam);
            return 0;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

static void Frame(imgui_app::UI& ui, settings::Settings& settings) {
    spdlog::trace("New frame...");
    static auto last_now = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - last_now).count();
    last_now = now;
    frametime_signal.update(1 / dt, dt);
    if (timer_find_game.tick(dt)) {
        // try find game if none found yet
        if (!game || (game && !is_process_running(game->handle.get()))) {
            game = find_game();
        };
        // try install hook if none installed yet
        if (game && !game->hook) {
            if (game->methods.hook_ready(game->handle.get(), game->base_ptrs)) {
                game->hook = game->methods.hook(game->handle, game->base_ptrs);
                if (!game->hook) {
                    // skip hooking, use stub...
                    game->hook = HookPtr{new Hook{}};
                }
                spdlog::info("Game is now tracked");
            } else {
                spdlog::debug("Game not yet ready for tracking...");
            }
        }
    }
    if (timer_update_stats.tick(dt)) {
        if (game && game->hook) {
            auto scoped_slow = ScopedProfiler{profiler_slow, dt};
            auto ok = game->methods.update_slow(
                game->exe_path,
                game->handle.get(),
                game->base_ptrs,
                game->hook->label_ptrs,
                game->remote_state,
                game->stats
            );
            error_slow.update(100.0f * static_cast<float>(!ok), dt);
        }
    }
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    auto& io = ImGui::GetIO();
    ImGui::SetNextWindowSize({io.DisplaySize.x, io.DisplaySize.y});
    ImGui::SetNextWindowPos({0, 0});
    if (ImGui::Begin(
            "Hitman Tracker",
            nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                | ImGuiWindowFlags_NoMove
        )) {
        ImGui::PushFont(ui.fonts[hitman_common::FontIndex::Settings]);
        if (ImGui::Button("Settings...")) {
            ImGui::OpenPopup("Settings");
        }
        if (ImGui::BeginPopup("Settings")) {
            g_settings_changed = settings_gui(settings);
            if (g_settings_changed.any) {
                settings::save(settings);
                g_settings_changed.any = false;
            }
            ImGui::EndPopup();
        }
        ImGui::PopFont();
        ImGui::Spacing();
        if (game && game->hook) {
            auto scoped_fast = ScopedProfiler{profiler_fast, dt};
            bool ok = game->methods.update_fast(
                game->handle.get(),
                game->base_ptrs,
                game->hook->label_ptrs,
                game->stats
            );
            error_fast.update(100.0f * static_cast<float>(!ok), dt);
            game->methods.gui(ui.fonts, game->stats);
        } else {
            imgui_app::text(
                ui.fonts[hitman_common::FontIndex::Title],
                im_vec4(settings.gui.title.color),
                "No game running"
            );
        }
    }
    ImGui::End();
    ImGui::EndFrame();
}

// Main code
int gui_run(settings::Settings& settings) {
    spdlog::info("Running user interface");
    ImGui_ImplWin32_EnableDpiAwareness();
    auto window_class
        = imgui_app::CreateWindowClass(WndProc, L"HitmanTrackerWindow");
    if (!window_class) return 1;
    auto window = imgui_app::CreateNativeWindow(
        window_class,
        L"Hitman Tracker",
        WS_OVERLAPPEDWINDOW,
        settings.gui.topmost ? WS_EX_TOPMOST : 0,
        15 * settings.gui.font_size,
        30 * settings.gui.font_size
    );
    if (!window) return 1;
    auto dev = imgui_app::CreateDeviceD3D(window->handle);
    if (!dev) return 1;

    spdlog::debug("Showing window...");
    ::ShowWindow(window->handle, SW_SHOWDEFAULT);
    ::UpdateWindow(window->handle);

    auto ui = imgui_app::CreateUI(
        *window,
        *dev,
        im_vec4(settings.gui.bg_color),
        hitman_common::make_font_specs(settings.gui)
    );
    if (!ui) return 1;

    // Main loop
    spdlog::debug("Starting main loop...");
    bool done = false;
    while (!done) {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the
        // Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;

        if (dev->state.is_lost) {
            spdlog::debug("Handling lost D3D device");
            HRESULT hr = dev->d3d_device->TestCooperativeLevel();
            if (hr == D3DERR_DEVICELOST) {
                spdlog::debug("Device still lost");
                ::Sleep(10);
                continue;
            }
            if (hr == D3DERR_DEVICENOTRESET) imgui_app::ResetDevice(*dev);
            spdlog::debug("Device recovered");
            dev->state.is_lost = false;
        }

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
            spdlog::debug("Handling window resize");
            dev->state.present_parameters.BackBufferWidth = g_ResizeWidth;
            dev->state.present_parameters.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            imgui_app::ResetDevice(*dev);
        }

        if (g_ChangeDpi != 0) {
            spdlog::debug("Handling dpi change");
            ::SetWindowPos(
                window->handle,
                NULL,
                g_ChangeRect.left,
                g_ChangeRect.top,
                g_ChangeRect.right - g_ChangeRect.left,
                g_ChangeRect.bottom - g_ChangeRect.top,
                SWP_NOZORDER
            );
            float dpiscale
                = (float)g_ChangeDpi / (float)USER_DEFAULT_SCREEN_DPI;
            g_ChangeDpi = 0;
            UpdateUIScaling(
                *ui,
                im_vec4(settings.gui.bg_color),
                dpiscale,
                hitman_common::make_font_specs(settings.gui)
            );
        }

        if (g_settings_changed.fonts) {
            float dpiscale = ImGui_ImplWin32_GetDpiScaleForHwnd(window->handle);
            UpdateUIScaling(
                *ui,
                im_vec4(settings.gui.bg_color),
                dpiscale,
                hitman_common::make_font_specs(settings.gui)
            );
            g_settings_changed.fonts = false;
        }

        if (g_settings_changed.topmost) {
            spdlog::debug("Topmost is {}", settings.gui.topmost);
            SetWindowPos(
                window->handle,
                settings.gui.topmost ? HWND_TOPMOST : HWND_NOTOPMOST,
                0,
                0,
                0,
                0,
                SWP_NOMOVE | SWP_NOSIZE
            );
            g_settings_changed.topmost = false;
        }

        Frame(*ui, settings);
        HRESULT result = imgui_app::RenderAndPresent(*dev);
    }
    spdlog::info("Closing user interface");
    spdlog::debug("Cleanup...");
    game.reset();

    return 0;
}