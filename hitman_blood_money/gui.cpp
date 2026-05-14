#include "gui.hpp"

#include "../hitman_common/gui.hpp"

const std::vector<std::string> map_names = {
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

void hitman_blood_money::gui(
    const settings::Gui& settings, const Fonts& fonts, const Stats& stats
) {
    hitman_common::gui(
        settings,
        fonts,
        stats,
        GAME_NAME,
        stats.difficulty == 0   ? "Rookie"
        : stats.difficulty == 1 ? "Normal"
        : stats.difficulty == 2 ? "Expert"
                                : "Pro",
        map_names,
        {
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
        }
    );
}
