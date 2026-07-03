// Dear ImGui: standalone example application for Windows API + DirectX 9

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/
// folder).
// - Introduction, links and more at the top of imgui.cpp

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include "gui/deviced3d.hpp"
#include "gui/window.hpp"

static bool g_DeviceLost = false;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int main(int argc, char** argv) {
    // Make process DPI aware
    ImGui_ImplWin32_EnableDpiAwareness();
    auto window = CreateWindowWin32(WndProc, 10, false);
    if (!window) return 1;
    auto dev = CreateDeviceD3D(window->handle);
    if (!dev) return 1;

    // Show the window
    ::ShowWindow(window->handle, SW_SHOWDEFAULT);
    ::UpdateWindow(window->handle);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags
        |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags
        |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    float dpiscale = ImGui_ImplWin32_GetDpiScaleForHwnd(window->handle);
    style.ScaleAllSizes(dpiscale);
    style.FontScaleDpi = dpiscale;
    if (!ImGui_ImplWin32_Init(window->handle)) return 0;
    if (!ImGui_ImplDX9_Init(dev->d3d_device)) return 0;

    // Load Fonts
    // - If fonts are not explicitly loaded, Dear ImGui will select an embedded
    // font: either AddFontDefaultVector() or AddFontDefaultBitmap().
    //   This selection is based on (style.FontSizeBase * style.FontScaleMain *
    //   style.FontScaleDpi) reaching a small threshold.
    // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to
    // select them.
    // - If a file cannot be loaded, AddFont functions will return a nullptr.
    // Please handle those errors in your code (e.g. use an assertion, display
    // an error and quit).
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use
    // FreeType for higher quality font rendering.
    // - Remember that in C/C++ if you want to include a backslash \ in a string
    // literal you need to write a double backslash \\ !
    // style.FontSizeBase = 20.0f;
    // io.Fonts->AddFontDefaultVector();
    // io.Fonts->AddFontDefaultBitmap();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    // ImFont* font =
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    // IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
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
            HRESULT hr = dev->d3d_device->TestCooperativeLevel();
            if (hr == D3DERR_DEVICELOST) {
                ::Sleep(10);
                continue;
            }
            if (hr == D3DERR_DEVICENOTRESET) ResetDevice(dev.get());
            g_DeviceLost = false;
        }

        // Handle window resize (we don't resize directly in the WM_SIZE
        // handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
            dev->d3d_present_parameters.BackBufferWidth = g_ResizeWidth;
            dev->d3d_present_parameters.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            ResetDevice(dev.get());
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in
        // ImGui::ShowDemoWindow()! You can browse its code to learn more about
        // Dear ImGui!).
        if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End
        // pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin(
                "Hello, world!"
            );  // Create a window called "Hello, world!" and append into it.

            ImGui::Text(
                "This is some useful text."
            );  // Display some text (you can use a format strings too)
            ImGui::Checkbox(
                "Demo Window", &show_demo_window
            );  // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat(
                "float", &f, 0.0f, 1.0f
            );  // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3(
                "clear color", (float*)&clear_color
            );  // Edit 3 floats representing a color

            if (ImGui::Button(
                    "Button"
                ))  // Buttons return true when clicked (most widgets return
                    // true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text(
                "Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / io.Framerate,
                io.Framerate
            );
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window) {
            ImGui::Begin(
                "Another Window", &show_another_window
            );  // Pass a pointer to our bool variable (the window will have a
                // closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me")) show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::EndFrame();
        dev->d3d_device->SetRenderState(D3DRS_ZENABLE, FALSE);
        dev->d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        dev->d3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(
            (int)(clear_color.x * clear_color.w * 255.0f),
            (int)(clear_color.y * clear_color.w * 255.0f),
            (int)(clear_color.z * clear_color.w * 255.0f),
            (int)(clear_color.w * 255.0f)
        );
        dev->d3d_device->Clear(
            0,
            nullptr,
            D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
            clear_col_dx,
            1.0f,
            0
        );
        if (dev->d3d_device->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            dev->d3d_device->EndScene();
        }
        HRESULT result
            = dev->d3d_device->Present(nullptr, nullptr, nullptr, nullptr);
        if (result == D3DERR_DEVICELOST) g_DeviceLost = true;
    }

    // Cleanup
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

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
            ::PostQuitMessage(0);
            return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}