#include "game_info.hpp"

#include "../game_info.hpp"
#include "../game_info_registry.hpp"
#include "gui.hpp"
#include "stats.hpp"

void hitman2_silent_assassin::register_game_info(
    const settings::Gui& settings
) {
    auto& registry = get_game_info_registry();
    registry.emplace_back(
        GameInfo{
            .name = GAME_NAME,
            .methods
            = GameMethods{gui(settings, "Steam"), hook_nothing, hook_immediately_ready, update_slow, update_fast},
            .module_infos = {{"hitman2.exe", PeId{0x3EF859D5}}},
        }
    );
};
