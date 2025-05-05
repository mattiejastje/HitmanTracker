#include "gui.hpp"

#include <imgui.h>

#include "../format_duration.hpp"

void hitman_blood_money::gui(const Stats& stats) {
    ImGui::Text("Hitman: Blood Money");
    ImGui::Separator();
    ImGui::Text(format_duration(stats.time).c_str());
}
