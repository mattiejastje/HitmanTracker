#pragma once

#include <imgui.h>

namespace imgui_app {

template <typename... Args>
void text(ImFont* font, const ImVec4& color, const char* fmt, Args... args) {
    if (font) ImGui::PushFont(font);
    ImGui::TextColored(color, fmt, args...);
    if (font) ImGui::PopFont();
}

}  // namespace imgui_app
