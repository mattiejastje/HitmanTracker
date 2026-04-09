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
#include "profiler.hpp"

// Data
constexpr auto TIMER_FIND_GAME = 1;
constexpr auto TIMER_UPDATE_STATS = 2;
static bool g_DeviceLost = false;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
static RECT g_ChangeRect = {};
static UINT g_ChangeDpi = 0;
static std::optional<Game> game{};
static Stats stats{0};
static Signal frametime_signal{"frame time", "seconds"};

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
                    if (!game
                        || (game && !is_process_running(game->handle.get()))) {
                        stats = {0};
                        game = find_game();
                    };
                    return 0;
                case TIMER_UPDATE_STATS:
                    auto now = std::chrono::steady_clock::now();
                    float dt
                        = std::chrono::duration<float>(now - last_now).count();
                    last_now = now;
                    if (game) {
                        game->methods.update_slow(
                            game->handle.get(),
                            game->base_ptrs,
                            game->hook->label_ptrs,
                            stats,
                            dt
                        );
                    };
                    return 0;
            }
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

static void Frame(UI* ui, const settings::Gui settings) {
    logging::trace("New frame...");
    static auto last_now = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - last_now).count();
    last_now = now;
    frametime_signal.update(dt, dt);
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
        if (game) {
            game->methods.update_fast(
                game->handle.get(),
                game->base_ptrs,
                game->hook->label_ptrs,
                stats,
                dt
            );
            game->methods.gui(settings, ui->fonts, stats);
        } else {
            text(ui->fonts.title, settings.title.color, "Game not running");
        }
    }
    ImGui::End();
    ImGui::EndFrame();
}

// Main code
int gui_run(const settings::Settings& settings) {
    ImGui_ImplWin32_EnableDpiAwareness();
    auto window = CreateWindowWin32(WndProc, settings.gui.font_size);
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
            UpdateUIScaling(ui.get(), dpiscale, settings.gui);
        }

        Frame(ui.get(), settings.gui);
        HRESULT result = RenderAndPresent(dev.get());
        if (result == D3DERR_DEVICELOST) g_DeviceLost = true;
    }
    logging::debug("Cleanup...");
    KillTimer(window->handle, TIMER_UPDATE_STATS);
    KillTimer(window->handle, TIMER_FIND_GAME);
    game.reset();

    return 0;
}