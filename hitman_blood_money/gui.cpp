#include "gui.hpp"

#include "../hitman_common/gui.hpp"

const std::vector<std::string> map_names = {
    "Hideout",                      // 1
    "#0 Death of a Showman",        // 2
    "#1 A Vintage Year",            // 3
    "#2 Curtains Down",             // 4
    "#3 Flatline",                  // 5
    "#4 A New Life",                // 6
    "#5 Murder of the Crows",       // 7
    "#6 You Better Watch Out",      // 8
    "#7 Death on the Mississippi",  // 9
    "#8 Till Death Do Us Part",     // 10
    "#9 A House of Cards",          // 11
    "#10 A Dance with The Devil",   // 12
    "#11 Amendment XXV",            // 13
    "#12 Requiem",                  // 14
};

void hitman_blood_money::gui(
    const settings::Gui& settings, const Fonts& fonts, const Stats& stats
) {
    hitman_common::gui(
        settings,
        fonts,
        stats,
        "Hitman: Blood Money",
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
