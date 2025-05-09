#include "gui.hpp"

#include <imgui.h>

#include "../format_duration.hpp"

void hitman_codename_47::gui(
    const settings::Gui& settings, const Fonts& fonts, const Stats& stats
) {
    ImGui::PushFont(fonts.title);
    ImGui::Text("Hitman: Codename 47");
    ImGui::PopFont();
    ImGui::Separator();
    ImGui::PushFont(fonts.time);
    ImGui::Text(format_duration(stats.time).c_str());
    ImGui::PopFont();
}
