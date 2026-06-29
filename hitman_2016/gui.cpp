#include "gui.hpp"

#include <format>

#include "../hitman_common/gui.hpp"

GameGui hitman_2016::gui(
    const settings::Gui& settings, const std::string& version
) {
    auto game_name = settings.show_game_version
                         ? std::format("{} [{}]", GAME_NAME, version)
                         : GAME_NAME;
    return [&settings, game_name](const Fonts& fonts, const Stats& stats) {
        hitman_common::gui(settings, fonts, 1.0f, stats, game_name, "", {}, {});
    };
}
