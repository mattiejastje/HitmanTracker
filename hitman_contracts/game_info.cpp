#include "game_info.hpp"

#include "../game_info.hpp"
#include "../game_info_registry.hpp"
#include "../hitman_common/stats.hpp"
#include "gui.hpp"
#include "stats.hpp"
#include "structs.hpp"

void hitman_contracts::register_game_info(const settings::Gui& settings) {
    auto& registry = get_game_info_registry();
    registry.emplace_back(
        GameInfo{
            .name = GAME_NAME,
            .methods
            = GameMethods{gui(settings, "Steam"), hook_nothing, hook_immediately_ready, update_slow(Version::Steam), update_fast(Version::Steam)},
            .make_remote_state
            = [] { return std::make_any<structs::HitmanContracts>(); },
            .make_stats = [] { return std::make_any<hitman_common::Stats>(); },
            .module_infos = {{"hitmancontracts.exe", PeId{0x52B2D458}}},
        }
    );
}
