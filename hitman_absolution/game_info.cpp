#include "game_info.hpp"

#include "../game_info.hpp"
#include "../game_info_registry.hpp"
#include "gui.hpp"
#include "hook.hpp"
#include "stats.hpp"

void hitman_absolution::register_game_info(
    const settings::Gui& settings, const settings::HMA& hma
) {
    auto& registry = get_game_info_registry();
    registry.emplace_back(
        GameInfo{
            .name = GAME_NAME,
            .methods
            = GameMethods{gui(settings, "Steam"), hook, hook_ready, update_slow(hma, GameVersion::Steam), update_fast},
            .module_infos = {{"hma.exe", PeId{0x5149E0B4}}},
        }
    );
    registry.emplace_back(
        GameInfo{
            .name = GAME_NAME,
            .methods
            = GameMethods{gui(settings, "GOG"), hook_nothing, hook_immediately_ready, stats_nothing_slow, stats_nothing_fast},
            .module_infos = {{"hma.exe", PeId{0x5C9A0BF7}}},
        }
    );
}
