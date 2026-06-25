#include "gui.hpp"

#include "../hitman_common/gui.hpp"

GameGui hitman_2016::gui(const settings::Gui& settings) {
    return [&settings](const Fonts& fonts, const Stats& stats) {
        hitman_common::gui(settings, fonts, 1.0f, stats, GAME_NAME, "", {}, {});
    };
}
