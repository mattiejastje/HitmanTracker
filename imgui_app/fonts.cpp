#include "fonts.hpp"

#include <spdlog/spdlog.h>

#include <map>
#include <ranges>

// direct load
[[nodiscard]] static ImFont* load_font(
    ImGuiIO& io, const std::filesystem::path& file, float size
) {
    auto im_font
        = std::filesystem::exists(file)
              ? io.Fonts->AddFontFromFileTTF(file.string().c_str(), size)
              : nullptr;
    if (im_font) {
        spdlog::debug("Font {} (size {}) loaded", file.string(), size);
    } else {
        spdlog::error(
            "Font {} (size {}) could not be loaded", file.string(), size
        );
    }
    return im_font;
}

using FontRegistry = std::map<imgui_app::FontSpec, ImFont*>;

// load with deduplication from registry
[[nodiscard]] static ImFont* load_font(
    ImGuiIO& io,
    FontRegistry& registry,
    const imgui_app::FontSpec& font,
    float dpiscale
) {
    auto iter = registry.find(font);
    if (iter != registry.end()) {
        return iter->second;
    } else {
        auto im_font = load_font(io, font.file, font.logical_size * dpiscale);
        registry[font] = im_font;
        return im_font;
    }
}

[[nodiscard]] std::vector<ImFont*> imgui_app::load_fonts(
    ImGuiIO& io, float dpiscale, std::span<const FontSpec> font_specs
) {
    io.Fonts->Clear();
    FontRegistry font_registry{};
    auto load = [&io, &font_registry, dpiscale](const FontSpec& font) {
        return load_font(io, font_registry, font, dpiscale);
    };
    return font_specs | std::views::transform(load)
           | std::ranges::to<std::vector>();
}