#include <spdlog/spdlog.h>

#include <iostream>

#include "game_info.hpp"
#include "gui.hpp"
#include "hitman2_silent_assassin/game_info.hpp"
#include "hitman_2016/game_info.hpp"
#include "hitman_absolution/game_info.hpp"
#include "hitman_blood_money/game_info.hpp"
#include "hitman_codename_47/game_info.hpp"
#include "hitman_contracts/game_info.hpp"
#include "settings.hpp"
#include "spdlog.hpp"

int main(int argc, char** argv) {
    spdlog_init();
    spdlog_set_level(4, 4);  // don't know levels yet, set to "info" for now
    auto settings = settings::load(argc, argv);
    if (settings) {
        spdlog_set_level(settings->log.level, settings->log.flush_level);
        spdlog::info("Hitman Tracker v" APP_VERSION);
        std::vector<GameInfo> registry;
        hitman_codename_47::register_game_info(registry, settings->gui);
        hitman2_silent_assassin::register_game_info(registry, settings->gui);
        hitman_contracts::register_game_info(registry, settings->gui);
        hitman_blood_money::register_game_info(
            registry, settings->gui, settings->hbm
        );
        hitman_absolution::register_game_info(
            registry, settings->gui, settings->hma
        );
        hitman_2016::register_game_info(registry, settings->gui);
        gui_run(registry, *settings);
    }
    return 0;
}
