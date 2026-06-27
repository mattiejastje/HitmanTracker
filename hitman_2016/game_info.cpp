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
                // steam dx11 version ("retail/HITMAN.exe")
                {"hitman.exe", PeId{0x5F8ED8B9}},
                {"tobii.gameintegration.dll", PeId{0x59BFC9A6}},
            },
        }
    );
}
