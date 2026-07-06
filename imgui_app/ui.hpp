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

bool UpdateUIScaling(
    UI& ui,
    ImVec4 bg_color,
    float dpiscale,
    std::span<const FontSpec> font_specs
);

}  // namespace imgui_app