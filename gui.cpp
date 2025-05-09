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
#include "gui/window.hpp"
#include "gui/deviced3d.hpp"
#include "gui/ui.hpp"
#include "imgui_utils.hpp"
#include "logging.hpp"

// Data
constexpr auto TIMER_FIND_GAME = 1;
constexpr auto TIMER_UPDATE_STATS = 2;
static bool g_DeviceLost = false;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
static std::optional<Game> game{};
static HookPtr hook{};
static Stats stats{0};

// Forward declarations of helper functions
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int gui_run(const settings::Settings& settings) {
    ImGui_ImplWin32_EnableDpiAwareness();
    auto window = CreateWindowWin32(WndProc, settings.gui.font_size);
    if (!window) return 1;
    auto dev = CreateDeviceD3D(window->handle);
    if (!dev) return 1;

    logging::trace("Showing window...");
    ::ShowWindow(window->handle, SW_SHOWDEFAULT);
    ::UpdateWindow(window->handle);

    auto ui = CreateUI(settings.gui, window.get(), dev.get());
    if (!ui) return 1;

    // Set timer
    SetTimer(window->handle, TIMER_FIND_GAME, 1000, nullptr);
    SetTimer(window->handle, TIMER_UPDATE_STATS, 100, nullptr);

    // Main loop
    logging::trace("Starting main loop...");
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

        // Handle lost D3D9 device
        if (g_DeviceLost) {
            HRESULT hr = dev->g_pd3dDevice->TestCooperativeLevel();
            if (hr == D3DERR_DEVICELOST) {
                ::Sleep(10);
                continue;
            }
            if (hr == D3DERR_DEVICENOTRESET) ResetDevice(ui.get(), dev.get());
            g_DeviceLost = false;
        }

        // Handle window resize (we don't resize directly in the WM_SIZE
        // handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
            dev->g_d3dpp.BackBufferWidth = g_ResizeWidth;
            dev->g_d3dpp.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            ResetDevice(ui.get(), dev.get());
        }

        // Start the frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        auto& io = ImGui::GetIO();
        ImGui::SetNextWindowSize({io.DisplaySize.x, io.DisplaySize.y});
        ImGui::SetNextWindowPos({0, 0});
        ImGui::Begin(
            "Hitman Tracker",
            nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                | ImGuiWindowFlags_NoMove
        );
        if (game) {
            auto target_ptr
                = hook ? (hook->target_alloc ? hook->target_alloc->ptr : 0) : 0;
            game->methods.update_fast(game->handle.get(), target_ptr, stats);
            game->methods.gui(settings.gui, ui->fonts, stats);
        } else {
            ImGui::Text("Game not running");
        }
        ImGui::End();
        ImGui::EndFrame();

        // Rendering
        dev->g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        dev->g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        dev->g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        if (dev->g_pd3dDevice->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            dev->g_pd3dDevice->EndScene();
        }
        HRESULT result
            = dev->g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
        if (result == D3DERR_DEVICELOST) g_DeviceLost = true;
    }
    // Cleanup
    logging::trace("Cleanup...");
    KillTimer(window->handle, TIMER_UPDATE_STATS);
    KillTimer(window->handle, TIMER_FIND_GAME);
    game.reset();

    return 0;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if
// dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your
// main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to
// your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from
// your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;

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
        case WM_DESTROY:
            game = {};
            hook = nullptr;
            stats = {0};
            ::PostQuitMessage(0);
            return 0;
        case WM_TIMER:
            switch (wParam) {
                case TIMER_FIND_GAME:
                    if (!game
                        || (game && !game_is_running(game->handle.get()))) {
                        hook = nullptr;
                        stats = {0};
                        game = find_game();
                        if (game) {
                            hook = game->methods.hook(game->handle);
                        };
                    };
                    return 0;
                case TIMER_UPDATE_STATS:
                    if (game) {
                        auto target_ptr = hook ? (hook->target_alloc
                                                      ? hook->target_alloc->ptr
                                                      : 0)
                                               : 0;
                        game->methods.update_slow(
                            game->handle.get(), target_ptr, stats
                        );
                    };
                    return 0;
            }
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
