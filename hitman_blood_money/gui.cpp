#include "gui.hpp"

#include "../hitman_common/gui.hpp"

const std::vector<std::string> map_names = {
    "Hideout",                   // 1
    "Death of a Showman",        // 2
    "A Vintage Year",            // 3
    "Curtains Down",             // 4
    "Flatline",                  // 5
    "A New Life",                // 6
    "Murder of the Crows",       // 7
    "You Better Watch Out",      // 8
    "Death on the Mississippi",  // 9
    "Till Death Do Us Part",     // 10
    "A House of Cards",          // 11
    "A Dance with The Devil",    // 12
    "Amendment XXV",             // 13
    "Requiem",                   // 14
};

void hitman_blood_money::gui(
    const settings::Gui& settings, const Fonts& fonts, const Stats& stats
) {
    hitman_common::gui(
        settings, fonts, stats, "Hitman: Blood Money", map_names, {}
    );
}
