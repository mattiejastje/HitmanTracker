#pragma once

#include <imgui.h>

template <typename... Args>
void text(
    ImFont* font, const ImVec4& color, const char* fmt, Args... args
) {
    if (font) ImGui::PushFont(font);
    ImGui::TextColored(color, fmt, args...);
    if (font) ImGui::PopFont();
}
