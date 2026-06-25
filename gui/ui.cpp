#include "ui.hpp"

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <filesystem>
#include <map>
#include <tuple>

#include "../imgui_utils.hpp"
#include "../logging.hpp"
#include "deviced3d.hpp"
#include "window.hpp"

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

void UIDeleter::operator()(UI* ui) const {
    if (ui) {
        logging::debug("Shutting down ImGui...");
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        if (ui->imgui_context) ImGui::DestroyContext(ui->imgui_context);
    }
}

std::unique_ptr<UI, UIDeleter> CreateUI(
    const settings::Gui& settings, Window* window, DeviceD3D* dev
) {
    logging::debug("Initializing ImGui...");
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
    if (!UpdateUIScaling(*ui, dpiscale, settings)) return nullptr;
    return ui;
}

bool UpdateUIScaling(UI& ui, float dpiscale, const settings::Gui& settings) {
    logging::debug("Updating UI for dpi scale {}...", dpiscale);
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
    style.Colors[ImGuiCol_WindowBg] = im_vec4(settings.bg_color);
    CopyMemory(&ImGui::GetStyle(), &style, sizeof(ImGuiStyle));

    // Load fonts
    io.Fonts->Clear();
    FontRegistry font_registry{};
    ui.fonts = Fonts{
        .settings = load_font(
            io, "fonts/proggyforever/ProggyForever-Regular.ttf", 10 * dpiscale
        ),
        .title = load_font(
            io,
            font_registry,
            get_font_key(settings.font_size * dpiscale, settings.title)
        ),
        .difficulty = load_font(
            io,
            font_registry,
            get_font_key(settings.font_size * dpiscale, settings.difficulty)
        ),
        .map = load_font(
            io,
            font_registry,
            get_font_key(settings.font_size * dpiscale, settings.map)
        ),
        .time = load_font(
            io,
            font_registry,
            get_font_key(settings.font_size * dpiscale, settings.time)
        ),
        .rating_bad = load_font(
            io,
            font_registry,
            get_font_key(settings.font_size * dpiscale, settings.rating_bad)
        ),
        .rating_good = load_font(
            io,
            font_registry,
            get_font_key(settings.font_size * dpiscale, settings.rating_good)
        ),
        .rating_maybe = load_font(
            io,
            font_registry,
            get_font_key(settings.font_size * dpiscale, settings.rating_maybe)
        ),
        .label = load_font(
            io,
            font_registry,
            get_font_key(settings.font_size * dpiscale, settings.label)
        ),
        .value = load_font(
            io,
            font_registry,
            get_font_key(settings.font_size * dpiscale, settings.value)
        ),
    };
    return ImGui_ImplDX9_CreateDeviceObjects();
};