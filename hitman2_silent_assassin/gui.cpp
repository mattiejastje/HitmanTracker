#include "gui.hpp"

#include <imgui.h>

#include "../format_duration.hpp"

void gui_hitman2_silent_assassin(const Stats& stats) {
    ImGui::Text("Hitman 2: Silent Assassin");
    ImGui::Separator();
    ImGui::Text(format_duration(stats.time).c_str());
}
