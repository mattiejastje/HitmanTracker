#include "gui.hpp"

#include <imgui.h>

#include <format>
#include <string>
#include <vector>

#include "../hitman_common/gui.hpp"
#include "../hitman_common/stats.hpp"
#include "../imgui_utils.hpp"

const std::vector<std::string> map_names = {
    "",
    "#1 Asylum Aftermath",
    "#2 The Meat King's Party",
    "#3 The Bjarkhov Bomb",
    "#4 Beldingford Manor",
    "#5 Rendezvous in Rotterdam",
    "#6 Deadly Cargo",
    "#7 Traditions of the Trade",
    "#8 Slaying a Dragon",
    "#9 The Wang Fou Incident",
    "#10 The Seafood Massacre",
    "#11 Lee Hong Assassination",
    "#12 Hunter and Hunted",
};

GameGui hitman_contracts::gui(
    const settings::Gui& settings, const std::string& version
) {
    return [&settings, version](const Fonts& fonts, const std::any& stats_any) {
        auto game_name = settings.show_game_version
                             ? std::format("{} [{}]", GAME_NAME, version)
                             : GAME_NAME;
        const auto& stats
            = std::any_cast<const hitman_common::Stats&>(stats_any);
        hitman_common::gui_header(
            settings,
            fonts,
            game_name,
            stats.difficulty == 1   ? "Normal"
            : stats.difficulty == 2 ? "Expert"
            : stats.difficulty == 3 ? "Professional"
                                    : "",
            map_names[stats.map],
            stats.map,
            stats.map_stage,
            stats.time
        );
        if (stats.map > 0) {
            hitman_common::gui_table(
                settings,
                fonts,
                stats.rating,
                stats.map,
                stats.map_stage,
                {
                    {"Close Encounters", stats.close_encounters},
                    {"Alerts", stats.alerts},
                    {"Shots Fired", stats.shots_fired},
                    {"Headshots", stats.headshots},
                    {"Enemies Killed", stats.enemies_killed},
                    {"Enemies Wounded", stats.enemies_wounded},
                    {"Innocents Killed", stats.innocents_killed},
                    {"Innocents Wounded", stats.innocents_wounded},
                }
            );
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