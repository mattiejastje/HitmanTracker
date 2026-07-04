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
