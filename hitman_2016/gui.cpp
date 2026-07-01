#include "gui.hpp"

#include <format>

#include "../hitman_common/gui.hpp"

GameGui hitman_2016::gui(
    const settings::Gui& settings, const std::string& version
) {
    return [&settings, version](const Fonts& fonts, const std::any& stats_any) {
        hitman_common::gui_header(
            settings, fonts, GAME_NAME, version, "", "", 0, MapStage::pre, 0.0f
        );
    };
}
