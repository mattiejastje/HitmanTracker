#include "ui.hpp"

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <map>
#include <ranges>
#include <tuple>

#include "../imgui_utils.hpp"
#include "deviced3d.hpp"
#include "window.hpp"


void UIDeleter::operator()(UI* ui) const {
    if (ui) {
        spdlog::debug("Shutting down ImGui...");
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        if (ui->imgui_context) ImGui::DestroyContext(ui->imgui_context);
    }
}

[[nodiscard]] std::unique_ptr<UI, UIDeleter> CreateUI(
    Window* window,
    DeviceD3D* dev,
    ImVec4 bg_color,
    std::span<const FontSpec> font_specs
) {
    spdlog::debug("Initializing ImGui...");
    auto ui = std::unique_ptr<UI, UIDeleter>(new UI());
    if (!IMGUI_CHECKVERSION()) return nullptr;
    ui->imgui_context = ImGui::CreateContext();
    if (!ui->imgui_context) return nullptr;
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    // Setup Platform/Renderer backends
    if (!ImGui_ImplWin32_Init(window->handle)) return nullptr;
    if (!ImGui_ImplDX9_Init(dev->d3d_device)) return nullptr;

    float dpiscale = ImGui_ImplWin32_GetDpiScaleForHwnd(window->handle);
    if (!UpdateUIScaling(*ui, bg_color, dpiscale, font_specs)) return nullptr;
    return ui;
}

bool UpdateUIScaling(
    UI& ui,
    ImVec4 bg_color,
    float dpiscale,
    std::span<const FontSpec> font_specs
) {
    spdlog::debug("Updating UI for dpi scale {}...", dpiscale);
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplDX9_InvalidateDeviceObjects();

    // Setup style
    auto style = ImGuiStyle();
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 0.0f;
    style.PopupBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;
    style.ScaleAllSizes(dpiscale);
    ImGui::StyleColorsDark(&style);
    style.Colors[ImGuiCol_WindowBg] = bg_color;
    CopyMemory(&ImGui::GetStyle(), &style, sizeof(ImGuiStyle));
    ui.fonts = load_fonts(io, dpiscale, font_specs);
    return ImGui_ImplDX9_CreateDeviceObjects();
};