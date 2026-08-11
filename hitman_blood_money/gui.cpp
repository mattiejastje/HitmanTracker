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
            "Hitman: Blood Money",
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
        std::vector<hitman_common::TableRow> table_rows{};
        if (hbm.merge_npcs) {
            table_rows.emplace_back(
                "NPCs Killed",
                merge_stats_values({
                    stats.innocents_killed,
                    stats.enemies_killed,
                    stats.police_killed,
                })
            );
            table_rows.emplace_back(
                "NPCs Wounded",
                merge_stats_values({
                    stats.innocents_wounded,
                    stats.enemies_wounded,
                    stats.police_wounded,
                })
            );
        } else {
            table_rows.emplace_back("Innocents Killed", stats.innocents_killed);
            table_rows.emplace_back(
                "Innocents Wounded", stats.innocents_wounded
            );
            table_rows.emplace_back("Enemies Killed", stats.enemies_killed);
            table_rows.emplace_back("Enemies Wounded", stats.enemies_wounded);
            table_rows.emplace_back("Police Killed", stats.police_killed);
            table_rows.emplace_back("Police Wounded", stats.police_wounded);
        }
        if (hbm.merge_frisk_cover) {
            table_rows.emplace_back(
                "Frisk Failed / Cover Blown",
                merge_stats_values({
                    stats.frisk_failed,
                    stats.cover_blown,
                })
            );
        } else {
            table_rows.emplace_back("Frisk Failed", stats.frisk_failed);
            table_rows.emplace_back("Cover Blown", stats.cover_blown);
        }
        if (hbm.merge_bodies_found) {
            table_rows.emplace_back(
                "Bodies Found",
                merge_stats_values({
                    stats.bodies_fnd,
                    stats.target_bodies_fnd,
                    stats.uncon_bodies_fnd,
                })
            );
        } else {
            table_rows.emplace_back("Bodies Found", stats.bodies_fnd);
            table_rows.emplace_back(
                "Target Bodies Found", stats.target_bodies_fnd
            );
            table_rows.emplace_back(
                "Unconscious Bodies Found", stats.uncon_bodies_fnd
            );
        }
        if (hbm.merge_witnesses_camera) {
            table_rows.emplace_back(
                "Witnesses / On Camera",
                merge_stats_values({
                    stats.witnesses,
                    stats.on_camera,
                })
            );
        } else {
            table_rows.emplace_back("Witnesses", stats.witnesses);
            table_rows.emplace_back("On Camera", stats.on_camera);
        }
        if (hbm.merge_items_left) {
            table_rows.emplace_back(
                "Items Left",
                merge_stats_values({
                    stats.cust_weapons_left,
                    stats.suit_left,
                })
            );
        } else {
            table_rows.emplace_back(
                "Custom Weapons Left", stats.cust_weapons_left
            );
            table_rows.emplace_back("Suit Left", stats.suit_left);
        }
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