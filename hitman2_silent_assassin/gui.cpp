#include "gui.hpp"

#include <imgui.h>

#include <string>
#include <vector>

#include "../format_duration.hpp"

const std::vector<std::string> map_names
    = {"The Gontranno Sactuary",    // 1
       "Anathema",                  // 2
       "St. Petersburg Stakeout",   // 3
       "Kirov Park Meeting",        // 4
       "Tubeway Torpedo",           // 5
       "Invitation to a Party",     // 6
       "Tracking Hayamoto",         // 7
       "Hidden Valley",             // 8
       "At the Gates",              // 9
       "Shogun Showdown",           // 10
       "Basement Killing",          // 11
       "The Graveyard Shift",       // 12
       "The Jacuzzi Job",           // 13
       "Murder at the Bazaar",      // 14
       "Motorcade Interception",    // 15
       "Tunnel Rat",                // 16
       "Temple City Ambush",        // 17
       "The Death of Hannelore",    // 18
       "Terminal Hospitality",      // 19
       "St. Petersburg Revisited",  // 20
       "Redemption at Gontranno"};  // 21

void hitman2_silent_assassin::gui(const Stats& stats) {
    ImGui::Text("Hitman 2: Silent Assassin");
    ImGui::Separator();
    if (stats.map > 0) {
        ImGui::Text("#%d %s", stats.map, map_names.at(stats.map - 1).c_str());
        ImGui::Text(format_duration(stats.time).c_str());
        ImGui::BeginTable("Statistics", 2);
        ImGui::TableNextColumn();
        ImGui::Text("Shots Fired");
        ImGui::Text("Close Encounters");
        ImGui::Text("Headshots");
        ImGui::Text("Alerts");
        ImGui::Text("Enemies Killed");
        ImGui::Text("Enemies Wounded");
        ImGui::Text("Innocents Killed");
        ImGui::Text("Innocents Wounded");
        ImGui::TableNextColumn();
        ImGui::Text("%d", stats.shots_fired);
        ImGui::Text("%d", stats.close_encounters);
        ImGui::Text("%d", stats.headshots);
        ImGui::Text("%d", stats.alerts);
        ImGui::Text("%d", stats.enemies_killed);
        ImGui::Text("%d", stats.enemies_wounded);
        ImGui::Text("%d", stats.innocents_killed);
        ImGui::Text("%d", stats.innocents_wounded);
        ImGui::EndTable();
    }
}
