#include "gui.hpp"

#include <imgui.h>

#include "../format_duration.hpp"

void hitman_codename_47::gui(const Stats& stats) {
    ImGui::Text("Hitman: Codename 47");
    ImGui::Separator();
    ImGui::Text(format_duration(stats.time).c_str());
}
