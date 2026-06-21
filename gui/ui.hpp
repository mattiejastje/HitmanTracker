#pragma once

#include <imgui.h>

#include <memory>

#include "../settings.hpp"
#include "deviced3d.hpp"
#include "window.hpp"

// imgui font for each text style in settings::Gui
struct Fonts {
    ImFont* title;
    ImFont* difficulty;
    ImFont* map;
    ImFont* time;
    ImFont* rating_bad;
    ImFont* rating_good;
    ImFont* rating_maybe;
    ImFont* label;
    ImFont* value;
};

struct UI {
    ImGuiContext* imgui_context;
    Fonts fonts;
};

struct UIDeleter {
    void operator()(UI* ui) const;
};

std::unique_ptr<UI, UIDeleter> CreateUI(
    const settings::Gui& settings, Window* window, DeviceD3D* dev
);

bool UpdateUIScaling(UI& ui, float dpiscale, const settings::Gui& settings);
