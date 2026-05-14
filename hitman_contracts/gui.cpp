#include "gui.hpp"

#include <imgui.h>

#include "../hitman_common/gui.hpp"
#include "../imgui_utils.hpp"

const std::vector<std::string> map_names = {
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

void hitman_contracts::gui(
    const settings::Gui& settings, const Fonts& fonts, const Stats& stats
) {
    hitman_common::gui(
        settings,
        fonts,
        stats,
        "Hitman: Contracts",
        stats.difficulty == 1   ? "Normal"
        : stats.difficulty == 2 ? "Expert"
        : stats.difficulty == 3 ? "Professional"
                                : "",
        map_names,
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
    if (stats.map > 0) {
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
                "%d",
                row.stats_value.value
            );
        }
        ImGui::EndTable();
    }
}
