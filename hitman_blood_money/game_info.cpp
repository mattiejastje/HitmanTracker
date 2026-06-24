#include "game_info.hpp"

#include "../game_info.hpp"
#include "../game_info_registry.hpp"
#include "gui.hpp"
#include "hook.hpp"
#include "stats.hpp"

void hitman_blood_money::register_game_info(const settings::Gui& settings) {
    auto& registry = get_game_info_registry();
    registry.emplace_back(
        GameInfo{
        .name = GAME_NAME,
        .methods = GameMethods{
            gui(settings),
            hook,
            hook_immediately_ready,
            update_slow,
            update_fast,
        },
        .module_infos = {{"hitmanbloodmoney.exe", 0xD31C7C7A7C311D9BULL}},
    });
}
