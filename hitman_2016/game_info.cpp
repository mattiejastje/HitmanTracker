#include "game_info.hpp"

#include "../game_info.hpp"
#include "../game_info_registry.hpp"
#include "gui.hpp"

void hitman_2016::register_game_info(const settings::Gui& settings) {
    auto& registry = get_game_info_registry();
    registry.emplace_back(
        GameInfo{
            .name = GAME_NAME,
            .methods = GameMethods{
                gui(settings),
                hook_nothing,
                hook_immediately_ready,
                stats_nothing_slow,
                stats_nothing_fast,
            },
            .module_infos = {
                {"hitman.exe", 0x9019923E9B36C383ULL},
                {"tobii.gameintegration.dll", 0xB36F82D72789C260ULL},
            },
        }
    );
}
