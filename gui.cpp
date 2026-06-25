#include "gui.hpp"

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <tchar.h>

#include <CLI/CLI.hpp>
#include <algorithm>
#include <filesystem>
#include <map>
#include <tuple>

#include "game.hpp"
#include "gui/deviced3d.hpp"
#include "gui/ui.hpp"
#include "gui/window.hpp"
#include "imgui_utils.hpp"
#include "logging.hpp"
#include "mem/handle.hpp"
#include "settings_gui.hpp"
#include "signal.hpp"

// Data
constexpr auto TIMER_FIND_GAME = 1;
constexpr auto TIMER_UPDATE_STATS = 2;
static bool g_DeviceLost = false;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
static RECT g_ChangeRect = {};
static UINT g_ChangeDpi = 0;
static std::optional<Game> game{};
static Stats stats{0};
static Signal frametime_signal{"fps", "frames per second"};
static Signal error_slow{"slow update failure rate", "%", 50.0f};
static Signal error_fast{"fast update failure rate", "%", 50.0f};
static Profiler profiler_slow{{"slow update time", "seconds"}};
static Profiler profiler_fast{{"fast update time", "seconds"}};
static bool g_show_settings = true;
static SettingsChanged g_settings_changed{};

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
);

// Win32 message handler
static LRESULT WINAPI
WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
    static auto last_now = std::chrono::steady_clock::now();
    switch (msg) {
        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED) return 0;
            g_ResizeWidth = (UINT)LOWORD(lParam);  // Queue resize
            g_ResizeHeight = (UINT)HIWORD(lParam);
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0)
                == SC_KEYMENU)  // Disable ALT application menu
                return 0;
            break;
        case WM_DPICHANGED:
            // Queue scaling update
            assert(LOWORD(wParam) == HIWORD(wParam));
            CopyMemory(&g_ChangeRect, (RECT*)lParam, sizeof(RECT));
            g_ChangeDpi = LOWORD(wParam);
            return 0;
        case WM_DESTROY:
            game = {};
            stats = {0};
            ::PostQuitMessage(0);
            return 0;
        case WM_TIMER:
            switch (wParam) {
                case TIMER_FIND_GAME:
                    // try find game if none found yet
                    if (!game
                        || (game && !is_process_running(game->handle.get()))) {
                        stats = {0};
                        game = find_game();
                    };
                    // try install hook if none installed yet
                    if (game && !game->hook) {
                        if (game->methods.hook_ready(
                                game->handle.get(), game->base_ptrs
                            )) {
                            game->hook = game->methods.hook(
                                game->handle, game->base_ptrs
                            );
                            if (!game->hook) {
                                // skip hooking, use stub...
                                game->hook = HookPtr{new Hook{}};
                            }
                            logging::info("Game is now tracked");
                        } else {
                            logging::info("Game not yet ready for tracking...");
                        }
                    }
                    return 0;
                case TIMER_UPDATE_STATS:
                    if (game && game->hook) {
                        auto now = std::chrono::steady_clock::now();
                        float dt = std::chrono::duration<float>(now - last_now)
                                       .count();
                        last_now = now;
                        auto scoped_slow = ScopedProfiler{profiler_slow, dt};
                        auto ok = game->methods.update_slow(
                            game->exe_path,
                            game->handle.get(),
                            game->base_ptrs,
                            game->hook->label_ptrs,
                            stats
                        );
                        error_slow.update(100.0f * static_cast<float>(!ok), dt);
                    }
                    return 0;
            }
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

static void Frame(UI& ui, settings::Settings& settings) {
    logging::trace("New frame...");
    static auto last_now = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - last_now).count();
    last_now = now;
    frametime_signal.update(1 / dt, dt);
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    auto& io = ImGui::GetIO();
    ImGui::SetNextWindowSize({io.DisplaySize.x, io.DisplaySize.y});
    ImGui::SetNextWindowPos({0, 0});
    if (ImGui::IsKeyPressed(ImGuiKey_F1)) {
        g_show_settings = !g_show_settings;
    }
    if (ImGui::Begin(
            "Hitman Tracker",
            nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                | ImGuiWindowFlags_NoMove
        )) {
        if (g_show_settings) {
            g_settings_changed = settings_gui(settings);
            if (g_settings_changed.any) {
                settings::save(settings);
                g_settings_changed.any = false;
            }
        }
        ImGui::Spacing();
        if (game && game->hook) {
            auto scoped_fast = ScopedProfiler{profiler_fast, dt};
            bool ok = game->methods.update_fast(
                game->handle.get(),
                game->base_ptrs,
                game->hook->label_ptrs,
                stats
            );
            error_fast.update(100.0f * static_cast<float>(!ok), dt);
            game->methods.gui(ui.fonts, stats);
        } else {
            text(ui.fonts.title, settings.gui.title.color, "No game running");
        }
    }
    ImGui::End();
    ImGui::EndFrame();
}

// Main code
int gui_run(settings::Settings& settings) {
    logging::info("Running user interface");
    g_show_settings = !settings.gui.hide_menu_on_start;
    ImGui_ImplWin32_EnableDpiAwareness();
    auto window = CreateWindowWin32(
        WndProc, settings.gui.font_size, settings.gui.topmost
    );
    if (!window) return 1;
    auto dev = CreateDeviceD3D(window->handle);
    if (!dev) return 1;

    logging::debug("Showing window...");
    ::ShowWindow(window->handle, SW_SHOWDEFAULT);
    ::UpdateWindow(window->handle);

    auto ui = CreateUI(settings.gui, window.get(), dev.get());
    if (!ui) return 1;

    // Set timer
    SetTimer(window->handle, TIMER_FIND_GAME, 1000, nullptr);
    SetTimer(window->handle, TIMER_UPDATE_STATS, 100, nullptr);

    // Main loop
    logging::debug("Starting main loop...");
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

        if (g_DeviceLost) {
            logging::debug("Handling lost D3D device");
            HRESULT hr = dev->d3d_device->TestCooperativeLevel();
            if (hr == D3DERR_DEVICELOST) {
                logging::debug("Device still lost");
                ::Sleep(10);
                continue;
            }
            if (hr == D3DERR_DEVICENOTRESET) ResetDevice(dev.get());
            logging::debug("Device recovered");
            g_DeviceLost = false;
        }

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
            logging::debug("Handling window resize");
            dev->d3d_present_parameters.BackBufferWidth = g_ResizeWidth;
            dev->d3d_present_parameters.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            ResetDevice(dev.get());
        }

        if (g_ChangeDpi != 0) {
            logging::debug("Handling dpi change");
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
            UpdateUIScaling(*ui, dpiscale, settings.gui);
        }

        if (g_settings_changed.fonts) {
            float dpiscale = ImGui_ImplWin32_GetDpiScaleForHwnd(window->handle);
            UpdateUIScaling(*ui, dpiscale, settings.gui);
            g_settings_changed.fonts = false;
        }

        if (g_settings_changed.topmost) {
            logging::debug("Topmost is {}", settings.gui.topmost);
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
        HRESULT result = RenderAndPresent(dev.get());
        if (result == D3DERR_DEVICELOST) g_DeviceLost = true;
    }
    logging::info("Closing user interface");
    logging::debug("Cleanup...");
    KillTimer(window->handle, TIMER_UPDATE_STATS);
    KillTimer(window->handle, TIMER_FIND_GAME);
    game.reset();

    return 0;
}