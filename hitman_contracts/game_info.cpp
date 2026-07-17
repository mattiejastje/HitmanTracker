#include "game_info.hpp"

#include "../hitman_common/stats.hpp"
#include "gui.hpp"
#include "stats.hpp"
#include "structs.hpp"

void hitman_contracts::register_game_info(
    std::vector<GameInfo>& registry, const settings::Gui& settings
) {
    registry.emplace_back(
        GameInfo{
            .tag = "hcon-s",
            .methods
            = GameMethods{gui(settings, "Steam Build 175"), hook_nothing, hook_immediately_ready, update_slow(Version::Steam), update_fast(Version::Steam)},
            .make_remote_state
            = [] { return std::make_any<structs::HitmanContracts>(); },
            .make_stats = [] { return std::make_any<hitman_common::Stats>(); },
            .module_infos = {{"hitmancontracts.exe", PeId{0x52B2D458}}},
        }
    );
    registry.emplace_back(
        GameInfo{
            .tag = "hcon-g",
            .methods
            = GameMethods{gui(settings, "GOG Build 174"), hook_nothing, hook_immediately_ready, update_slow(Version::GOG), update_fast(Version::GOG)},
            .make_remote_state
            = [] { return std::make_any<structs::HitmanContracts>(); },
            .make_stats = [] { return std::make_any<hitman_common::Stats>(); },
            .module_infos = {{"hitmancontracts.exe", PeId{0x4091141B}}},
        }
    );
}
