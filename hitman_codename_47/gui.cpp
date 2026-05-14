#include "gui.hpp"

#include <imgui.h>

#include "../hitman_common/gui.hpp"

const std::vector<std::string> map_names = {
    "#1 Training",
    "#2 Kowloon Triads in Gang War",
    "#3 Ambush at the Wang Fou Restaurant",
    "#4 The Massacre at Cheung Chau Fish Restaurant",
    "#5 The Lee Hong Assassination",
    "#6 Find the U'wa Tribe",
    "#7 The Jungle God",
    "#8 Say Hello to my Little Friend",
    "#9 Traditions of the Trade",
    "#10 Gunrunner's Paradise",
    "#11 Plutonium Runs Loose",
    "#12 The Setup",
    "#13 Meet Your Brother",
};

void hitman_codename_47::gui(
    const settings::Gui& settings, const Fonts& fonts, const Stats& stats
) {
    hitman_common::gui(
        settings,
        fonts,
        stats,
        GAME_NAME,
        stats.difficulty == 0   ? "Easy"
        : stats.difficulty == 1 ? "Normal"
        : stats.difficulty == 2 ? "Hard"
                                : "",
        map_names,
        {}
    );
}
