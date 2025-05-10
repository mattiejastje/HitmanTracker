#include "gui.hpp"

#include <imgui.h>

#include <array>
#include <string>
#include <vector>

#include "../format_duration.hpp"
#include "../imgui_utils.hpp"

const std::vector<std::string> map_names
    = {"The Gontranno Sactuary",      // 1
       "Anathema",                    // 2
       "St. Petersburg Stakeout",     // 3
       "Kirov Park Meeting",          // 4
       "Tubeway Torpedo",             // 5
       "Invitation to a Party",       // 6
       "Tracking Hayamoto",           // 7
       "Hidden Valley",               // 8
       "At the Gates",                // 9
       "Shogun Showdown",             // 10
       "Basement Killing",            // 11
       "The Graveyard Shift",         // 12
       "The Jacuzzi Job",             // 13
       "Murder at the Bazaar",        // 14
       "The Motorcade Interception",  // 15
       "Tunnel Rat",                  // 16
       "Temple City Ambush",          // 17
       "The Death of Hannelore",      // 18
       "Terminal Hospitality",        // 19
       "St. Petersburg Revisited",    // 20
       "Redemption at Gontranno"};    // 21

void hitman2_silent_assassin::gui(
    const settings::Gui& settings, const Fonts& fonts, const Stats& stats
) {
    text(fonts.title, settings.title.color, "Hitman 2: Silent Assassin");
    if (stats.map > 0) {
        text(
            fonts.map,
            settings.map.color,
            "#%d %s",
            stats.map,
            map_names.at(stats.map - 1).c_str()
        );
        ImGui::Spacing();
        text(
            fonts.time, settings.time.color, format_duration(stats.time).c_str()
        );
        auto rating_font
            = stats.silent_assassin == Status::RED     ? fonts.rating_bad
              : stats.silent_assassin == Status::GREEN ? fonts.rating_good
                                                       : fonts.rating_maybe;
        auto rating_color = stats.silent_assassin == Status::RED
                                ? settings.rating_bad.color
                            : stats.silent_assassin == Status::GREEN
                                ? settings.rating_good.color
                                : settings.rating_maybe.color;
        auto rating_text = stats.silent_assassin == Status::RED
                               ? "No Silent Assassin"
                               : "Silent Assassin";
        text(rating_font, rating_color, rating_text);
        ImGui::Spacing();
        ImGui::BeginTable(
            "Statistics",
            2,
            ImGuiTableFlags_SizingFixedFit
                | ImGuiTableFlags_NoKeepColumnsVisible
                | ImGuiTableFlags_NoHostExtendX
        );
        table_row(
            fonts,
            settings,
            stats.shots_fired.status,
            "Shots Fired",
            "%d",
            stats.shots_fired.value
        );
        table_row(
            fonts,
            settings,
            stats.close_encounters.status,
            "Close Encounters",
            "%d",
            stats.close_encounters.value
        );
        table_row(
            fonts,
            settings,
            stats.headshots.status,
            "Headshots",
            "%d",
            stats.headshots.value
        );
        table_row(
            fonts,
            settings,
            stats.alerts.status,
            "Alerts",
            "%d",
            stats.alerts.value
        );
        table_row(
            fonts,
            settings,
            stats.enemies_killed.status,
            "Enemies Killed",
            "%d",
            stats.enemies_killed.value
        );
        table_row(
            fonts,
            settings,
            stats.enemies_wounded.status,
            "Enemies Wounded",
            "%d",
            stats.enemies_wounded.value
        );
        table_row(
            fonts,
            settings,
            stats.innocents_killed.status,
            "Innocents Killed",
            "%d",
            stats.innocents_killed.value
        );
        table_row(
            fonts,
            settings,
            stats.innocents_wounded.status,
            "Innocents Wounded",
            "%d",
            stats.innocents_wounded.value
        );
        ImGui::EndTable();
    }
}
