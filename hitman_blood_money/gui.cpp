#include "gui.hpp"

#include <format>

#include "../hitman_common/gui.hpp"
#include "../imgui_app/text.hpp"
#include "stats.hpp"

const std::vector<std::string> map_names = {
    "",
    "Hideout",                      // 1
    "#1 Death of a Showman",        // 2
    "#2 A Vintage Year",            // 3
    "#3 Curtains Down",             // 4
    "#4 Flatline",                  // 5
    "#5 A New Life",                // 6
    "#6 Murder of the Crows",       // 7
    "#7 You Better Watch Out",      // 8
    "#8 Death on the Mississippi",  // 9
    "#9 Till Death Do Us Part",     // 10
    "#10 A House of Cards",         // 11
    "#11 A Dance with The Devil",   // 12
    "#12 Amendment XXV",            // 13
    "#13 Requiem",                  // 14
};

GameGui hitman_blood_money::gui(
    const settings::Gui& settings,
    const settings::HBM& hbm,
    const std::string& version
) {
    return [&settings,
            &hbm,
            version](std::span<ImFont*> fonts, const std::any& stats_any) {
        const auto& stats = std::any_cast<const Stats&>(stats_any);
        hitman_common::gui_header(
            settings,
            fonts,
            GAME_NAME,
            version,
            stats.difficulty == 0   ? "Rookie"
            : stats.difficulty == 1 ? "Normal"
            : stats.difficulty == 2 ? "Expert"
                                    : "Pro",
            map_names[stats.map],
            stats.map,
            stats.map_stage,
            stats.time * (hbm.real_time ? (1000 / 1024.0f) : 1.0f)
        );
        std::vector<hitman_common::TableRow> table_rows = {
            {"Innocents Killed", stats.innocents_killed},
            {"Innocents Wounded", stats.innocents_wounded},
            {"Enemies Killed", stats.enemies_killed},
            {"Enemies Wounded", stats.enemies_wounded},
            {"Police Killed", stats.police_killed},
            {"Police Wounded", stats.police_wounded},
            {"Frisk Failed", stats.frisk_failed},
            {"Cover Blown", stats.cover_blown},
            {"Bodies Found", stats.bodies_fnd},
            {"Target Bodies Found", stats.target_bodies_fnd},
            {"Unconscious Bodies Found", stats.uncon_bodies_fnd},
            {"Witnesses", stats.witnesses},
            {"On Camera", stats.on_camera},
            {"Custom Weapons Left", stats.cust_weapons_left},
            {"Suit Left", stats.suit_left},
        };
        if (hbm.show_accident_kills) {
            table_rows.emplace_back(
                "Accident Kills", StatsValue{stats.accident_kills}
            );
        }
        if (hbm.show_shots_hit) {
            table_rows.emplace_back("Shots Hit", StatsValue{stats.shots_hit});
        }
        hitman_common::gui_table(
            settings,
            fonts,
            "sa_rating",
            stats.rating,
            stats.map,
            stats.map_stage,
            table_rows
        );
    };
}