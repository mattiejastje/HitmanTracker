#include "game_info.hpp"

#include "../game_info.hpp"
#include "../game_info_registry.hpp"
#include "gui.hpp"
#include "stats.hpp"

void hitman_codename_47::register_game_info(const settings::Gui& settings) {
    auto& registry = get_game_info_registry();
    registry.emplace_back(GameInfo{
        .name = GAME_NAME,
        .methods = GameMethods{
            gui(settings),
            hook_nothing,
            hook_immediately_ready,
            update_slow,
            update_fast,
        },
        .module_infos = {
            {"hitman.exe", PeId{0x3A3E13DA}},
            {"hitmandlc.dlc", PeId{0x3A3E13D1}},
            {"enginedata.dll", PeId{0x3A3E12F7}},
        },
    });
}
