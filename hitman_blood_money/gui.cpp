#include "gui.hpp"

#include <imgui.h>

#include "../format_duration.hpp"

void gui_hitman_blood_money(const Stats& stats) {
    ImGui::Text("Hitman: Blood Money");
    ImGui::Separator();
    ImGui::Text(format_duration(stats.time).c_str());
}
