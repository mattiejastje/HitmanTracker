#include "text.hpp"

void imgui_app::text(ImFont* font, const ImVec4& color, const char* msg) {
    if (font) ImGui::PushFont(font);
    ImGui::TextColored(color, "%s", msg);
    if (font) ImGui::PopFont();
}
