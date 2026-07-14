#include "ui.hpp"

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <map>
#include <ranges>
#include <tuple>

#include "deviced3d.hpp"
#include "window.hpp"

void imgui_app::UIDeleter::operator()(UI* ui) const {
    if (ui) {
        spdlog::debug("Shutting down ImGui...");
        if (ui->imgui_context) {
            ImGui::SetCurrentContext(ui->imgui_context);
            ImGui_ImplDX9_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext(ui->imgui_context);
        }
        delete ui;
    }
}

imgui_app::UIPtr imgui_app::CreateUI(
    Window& window,
    DeviceD3D& dev,
    ImVec4 bg_color,
    std::span<const FontSpec> font_specs
) {
    spdlog::debug("Initializing ImGui...");
    if (!IMGUI_CHECKVERSION()) return nullptr;
    auto* context = ImGui::CreateContext();
    if (!context) return nullptr;
    ImGui::SetCurrentContext(context);
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    if (!ImGui_ImplWin32_Init(window.handle)) {
        ImGui::DestroyContext(context);
        return nullptr;
    }
    if (!ImGui_ImplDX9_Init(dev.d3d_device)) {
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext(context);
        return nullptr;
    }
    auto ui = UIPtr{new UI{
        .imgui_context = context,
        .fonts = {},
        .bg_color = bg_color,
        .font_specs = {font_specs.begin(), font_specs.end()}
    }};
    float dpiscale = ImGui_ImplWin32_GetDpiScaleForHwnd(window.handle);
    if (!UpdateUIScaling(*ui, dpiscale)) return nullptr;
    return ui;
}

[[nodiscard]] bool imgui_app::UpdateUIScaling(UI& ui, float dpiscale) {
    spdlog::debug("Updating UI for dpi scale {}...", dpiscale);
    ImGui::SetCurrentContext(ui.imgui_context);
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplDX9_InvalidateDeviceObjects();
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
    style.Colors[ImGuiCol_WindowBg] = ui.bg_color;
    CopyMemory(&ImGui::GetStyle(), &style, sizeof(ImGuiStyle));
    ui.fonts = load_fonts(io, dpiscale, ui.font_specs);
    auto ok = ImGui_ImplDX9_CreateDeviceObjects();
    if (!ok) spdlog::critical("Failed to create device objects");
    return ok;
};