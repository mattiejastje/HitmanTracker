#pragma once

#include <imgui.h>

#include <array>
#include <optional>

#include "game.hpp"
#include "settings.hpp"

ImVec4 im_vec4(const std::array<float, 3>& color);

template <typename... Args>
void text(
    ImFont* font, std::array<float, 3> color, const char* fmt, Args... args
) {
    if (font) ImGui::PushFont(font);
    ImGui::TextColored(im_vec4(color), fmt, args...);
    if (font) ImGui::PopFont();
}

template <typename... Args>
void table_row(
    ImFont* label_font,
    std::array<float, 3> label_color,
    ImFont* value_font,
    std::array<float, 3> value_color,
    const char* label,
    const char* value,
    Args... args
) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    text(value_font, value_color, value, args...);
    ImGui::TableNextColumn();
    text(label_font, label_color, label);
}

template <typename... Args>
void table_row(
    const Fonts& fonts,
    const settings::Gui& settings,
    std::optional<Status> status,
    const char* label,
    const char* value,
    Args... args
) {
    auto value_color
        = !status.has_value()               ? settings.value.color
          : status.value() == Status::GREEN ? settings.rating_good.color
          : status.value() == Status::RED   ? settings.rating_bad.color
                                            : settings.rating_maybe.color;
    table_row(
        fonts.label,
        settings.label.color,
        fonts.value,
        value_color,
        label,
        value,
        args...
    );
}
