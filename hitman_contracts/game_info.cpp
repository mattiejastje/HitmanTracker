#include "game_info.hpp"

#include "../game_info.hpp"
#include "../game_info_registry.hpp"
#include "gui.hpp"
#include "stats.hpp"

void hitman_contracts::register_game_info(const settings::Gui& settings) {
    auto& registry = get_game_info_registry();
    registry.emplace_back(
        GameInfo{
            .name = GAME_NAME,
            .methods
            = GameMethods{gui(settings, "Steam"), hook_nothing, hook_immediately_ready, update_slow, update_fast},
            .module_infos = {{"hitmancontracts.exe", PeId{0x52B2D458}}},
        }
    );
}
