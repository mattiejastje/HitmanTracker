#include "gui.hpp"

#include <imgui.h>

#include "../format_duration.hpp"

void gui_hitman_contracts(const Stats& stats) {
    ImGui::Text("Hitman: Contracts");
    ImGui::Separator();
    ImGui::Text(format_duration(stats.time).c_str());
}
