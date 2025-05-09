#include "gui.hpp"

#pragma comment(lib, "d3d9.lib")

#include <d3d9.h>
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
#include "imgui_utils.hpp"
#include "logging.hpp"

// Data
constexpr auto TIMER_FIND_GAME = 1;
constexpr auto TIMER_UPDATE_STATS = 2;
static LPDIRECT3D9 g_pD3D = nullptr;
static LPDIRECT3DDEVICE9 g_pd3dDevice = nullptr;
static bool g_DeviceLost = false;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS g_d3dpp = {};
static std::optional<Game> game{};
static HookPtr hook{};
static Stats stats{0};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static ImFont* load_font(
    ImGuiIO& io, const std::filesystem::path file, float size
) {
    auto im_font
        = std::filesystem::exists(file)
              ? io.Fonts->AddFontFromFileTTF(file.string().c_str(), size)
              : nullptr;
    if (im_font) {
        logging::debug("Font {} (size {}) loaded", file.string(), size);
    } else {
        logging::error(
            "Font {} (size {}) could not be loaded", file.string(), size
        );
    }
    return im_font;
}

using FontKey = std::tuple<std::filesystem::path, float>;
using FontRegistry = std::map<FontKey, ImFont*>;

static ImFont* load_font(
    ImGuiIO& io, FontRegistry& registry, const FontKey& key
) {
    auto iter = registry.find(key);
    if (iter != registry.end()) {
        return iter->second;
    } else {
        auto im_font = load_font(io, std::get<0>(key), std::get<1>(key));
        registry[key] = im_font;
        return im_font;
    }
}

static FontKey get_font_key(float font_size, const settings::TextStyle& style) {
    return {style.file, font_size * style.scale};
}

// Main code
int gui_run(const settings::Settings& settings) {
    // Create application window
    logging::trace("Creating application window...");
    WNDCLASSEXW wc
        = {sizeof(wc),
           CS_CLASSDC,
           WndProc,
           0L,
           0L,
           GetModuleHandle(nullptr),
           nullptr,
           nullptr,
           nullptr,
           nullptr,
           L"Hitman Tracker",
           nullptr};
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(
        wc.lpszClassName,
        L"Hitman Tracker",
        WS_OVERLAPPEDWINDOW,
        100,
        100,
        15 * settings.gui.font_size,
        30 * settings.gui.font_size,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr
    );

    // Initialize Direct3D
    logging::trace("Initializing Direct3D...");
    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    logging::trace("Showing window...");
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    logging::trace("Initializing ImGui...");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    auto& style = ImGui::GetStyle();
    style.WindowBorderSize = 0.0f;
    style.Colors[ImGuiCol_WindowBg] = im_vec4(settings.gui.bg_color);

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    // Load fonts
    FontRegistry font_registry{};
    Fonts fonts{
        .title = load_font(
            io,
            font_registry,
            get_font_key(settings.gui.font_size, settings.gui.title)
        ),
        .map = load_font(
            io,
            font_registry,
            get_font_key(settings.gui.font_size, settings.gui.map)
        ),
        .time = load_font(
            io,
            font_registry,
            get_font_key(settings.gui.font_size, settings.gui.time)
        ),
        .rating_bad = load_font(
            io,
            font_registry,
            get_font_key(settings.gui.font_size, settings.gui.rating_bad)
        ),
        .rating_good = load_font(
            io,
            font_registry,
            get_font_key(settings.gui.font_size, settings.gui.rating_good)
        ),
        .rating_maybe = load_font(
            io,
            font_registry,
            get_font_key(settings.gui.font_size, settings.gui.rating_maybe)
        ),
        .label = load_font(
            io,
            font_registry,
            get_font_key(settings.gui.font_size, settings.gui.label)
        ),
        .value = load_font(
            io,
            font_registry,
            get_font_key(settings.gui.font_size, settings.gui.value)
        ),
    };

    // Set timer
    SetTimer(hwnd, TIMER_FIND_GAME, 1000, nullptr);
    SetTimer(hwnd, TIMER_UPDATE_STATS, 100, nullptr);

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
            HRESULT hr = g_pd3dDevice->TestCooperativeLevel();
            if (hr == D3DERR_DEVICELOST) {
                ::Sleep(10);
                continue;
            }
            if (hr == D3DERR_DEVICENOTRESET) ResetDevice();
            g_DeviceLost = false;
        }

        // Handle window resize (we don't resize directly in the WM_SIZE
        // handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
            g_d3dpp.BackBufferWidth = g_ResizeWidth;
            g_d3dpp.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            ResetDevice();
        }

        // Start the frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
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
            game->methods.gui(settings.gui, fonts, stats);
        } else {
            ImGui::Text("Game not running");
        }
        ImGui::End();
        ImGui::EndFrame();

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        if (g_pd3dDevice->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result
            = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
        if (result == D3DERR_DEVICELOST) g_DeviceLost = true;
    }

    // Cleanup
    logging::trace("Cleanup...");
    KillTimer(hwnd, TIMER_UPDATE_STATS);
    KillTimer(hwnd, TIMER_FIND_GAME);
    game.reset();
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd) {
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr) return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat
        = D3DFMT_UNKNOWN;  // Need to use an explicit format with alpha if
                           // needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval
        = D3DPRESENT_INTERVAL_ONE;  // Present with vsync
    // g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   //
    // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL,
            hWnd,
            D3DCREATE_HARDWARE_VERTEXPROCESSING,
            &g_d3dpp,
            &g_pd3dDevice
        )
        < 0)
        return false;

    return true;
}

void CleanupDeviceD3D() {
    if (g_pd3dDevice) {
        g_pd3dDevice->Release();
        g_pd3dDevice = nullptr;
    }
    if (g_pD3D) {
        g_pD3D->Release();
        g_pD3D = nullptr;
    }
}

void ResetDevice() {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL) IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
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
