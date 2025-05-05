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

static void table_row(const char* text, const char* fmt, ...) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text(text);
    ImGui::TableNextColumn();
    va_list args;
    va_start(args, fmt);
    ImGui::TextV(fmt, args);
    va_end(args);
}

void hitman2_silent_assassin::gui(const Stats& stats) {
    ImGui::Text("Hitman 2: Silent Assassin");
    ImGui::Separator();
    if (stats.map > 0) {
        ImGui::Text("#%d %s", stats.map, map_names.at(stats.map - 1).c_str());
        ImGui::Text(format_duration(stats.time).c_str());
        ImGui::BeginTable("Statistics", 2, ImGuiTableFlags_BordersH | ImGuiTableFlags_SizingFixedFit);
        table_row(
            "Silent Assassin",
            stats.silent_assassin == SilentAssassin::YES     ? "Yes"
            : stats.silent_assassin == SilentAssassin::MAYBE ? "Maybe"
                                                             : "No"
        );
        table_row("Shots Fired", "%d", stats.shots_fired);
        table_row("Close Encounters", "%d", stats.close_encounters);
        table_row("Headshots", "%d", stats.headshots);
        table_row("Alerts", "%d", stats.alerts);
        table_row("Enemies Killed", "%d", stats.enemies_killed);
        table_row("Enemies Wounded", "%d", stats.enemies_wounded);
        table_row("Innocents Killed", "%d", stats.innocents_killed);
        table_row("Innocents Wounded", "%d", stats.innocents_wounded);
        ImGui::EndTable();
    }
}
