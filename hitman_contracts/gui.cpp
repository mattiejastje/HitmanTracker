#include "gui.hpp"

#include <imgui.h>

#include "../format_duration.hpp"

void hitman_contracts::gui(const Stats& stats) {
    ImGui::Text("Hitman: Contracts");
    ImGui::Separator();
    ImGui::Text(format_duration(stats.time).c_str());
}
