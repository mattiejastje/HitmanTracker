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
            {"hitman.exe", 0xD6739CF25081C0F5ULL},
            {"hitmandlc.dlc", 0xCC2D12E73040901FULL},
            {"enginedata.dll", 0xA0C506C5C1D98559ULL},
        },
    });
}
