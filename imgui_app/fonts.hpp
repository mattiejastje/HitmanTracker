#pragma once

#include <imgui.h>

#include <filesystem>
#include <span>
#include <vector>

namespace imgui_app {

struct FontSpec {
    std::filesystem::path file;
    float logical_size;
    auto operator<=>(const FontSpec&) const = default;
};

[[nodiscard]] std::vector<ImFont*> load_fonts(
    ImGuiIO& io, float dpiscale, std::span<const FontSpec> specs
);

}  // namespace imgui_app