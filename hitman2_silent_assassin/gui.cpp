#include "gui.hpp"

#include <imgui.h>

#include <format>
#include <string>
#include <vector>

#include "../hitman_common/gui.hpp"
#include "../imgui_utils.hpp"

const std::vector<std::string> map_names = {
    "The Gontranno Sanctuary",         // 1
    "#1 Anathema",                     // 2
    "#2 St. Petersburg Stakeout",      // 3
    "#3 Kirov Park Meeting",           // 4
    "#4 Tubeway Torpedo",              // 5
    "#5 Invitation to a Party",        // 6
    "#6 Tracking Hayamoto",            // 7
    "#7 Hidden Valley",                // 8
    "#8 At the Gates",                 // 9
    "#9 Shogun Showdown",              // 10
    "#10 Basement Killing",            // 11
    "#11 The Graveyard Shift",         // 12
    "#12 The Jacuzzi Job",             // 13
    "#13 Murder at the Bazaar",        // 14
    "#14 The Motorcade Interception",  // 15
    "#15 Tunnel Rat",                  // 16
    "#16 Temple City Ambush",          // 17
    "#17 The Death of Hannelore",      // 18
    "#18 Terminal Hospitality",        // 19
    "#19 St. Petersburg Revisited",    // 20
    "#20 Redemption at Gontranno",     // 21
};

GameGui hitman2_silent_assassin::gui(
    const settings::Gui& settings, const std::string& version
) {
    auto game_name = settings.show_game_version
                         ? std::format("{} [{}]", GAME_NAME, version)
                         : GAME_NAME;
    return [&settings, game_name](const Fonts& fonts, const Stats& stats) {
        hitman_common::gui(
        settings,
        fonts,
        1.0f,
        stats,
        game_name,
        stats.difficulty == 1   ? "Normal"
        : stats.difficulty == 2 ? "Expert"
        : stats.difficulty == 3 ? "Professional"
                                : "",
        map_names,
        stats.map >= 2 ?
        std::vector<hitman_common::TableRow>{
            {"Close Encounters", stats.close_encounters},
            {"Alerts", stats.alerts},
            {"Shots Fired", stats.shots_fired},
            {"Headshots", stats.headshots},
            {"Enemies Killed", stats.enemies_killed},
            {"Enemies Wounded", stats.enemies_wounded},
            {"Innocents Killed", stats.innocents_killed},
            {"Innocents Wounded", stats.innocents_wounded},
        } : std::vector<hitman_common::TableRow>{}
    );
        if (stats.map >= 2) {
            ImGui::Spacing();
            ImGui::BeginTable(
                "Statistics",
                2,
                ImGuiTableFlags_SizingFixedFit
                    | ImGuiTableFlags_NoKeepColumnsVisible
                    | ImGuiTableFlags_NoHostExtendX
            );
            std::vector<hitman_common::TableRow> table_rows = {
                {"Stealth", stats.stealth},
                {"Aggression", stats.aggression},
            };
            for (auto& row : table_rows) {
                table_row(
                    fonts,
                    settings,
                    row.stats_value.status,
                    row.name.c_str(),
                    "%.3g",
                    row.stats_value.value / 10.0
                );
            }
            ImGui::EndTable();
        }
    };
}
