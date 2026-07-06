#pragma once

#include <imgui.h>

#include <span>

#include "deviced3d.hpp"
#include "fonts.hpp"
#include "window.hpp"

namespace imgui_app {

struct UI {
    ImGuiContext* imgui_context;
    std::vector<ImFont*> fonts;
    // stored here so can be easily recalled by UpdateUIScaling
    ImVec4 bg_color;
    std::vector<FontSpec> font_specs;
};

struct UIDeleter {
    void operator()(UI* ui) const;
};

using UIPtr = std::unique_ptr<UI, UIDeleter>;

[[nodiscard]] UIPtr CreateUI(
    Window& window,
    DeviceD3D& dev,
    ImVec4 bg_color,
    std::span<const FontSpec> fonts
);

bool UpdateUIScaling(UI& ui, float dpiscale);

}  // namespace imgui_app