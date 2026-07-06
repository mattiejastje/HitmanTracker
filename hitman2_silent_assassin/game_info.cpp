#include "game_info.hpp"

#include "../hitman_common/stats.hpp"
#include "gui.hpp"
#include "stats.hpp"
#include "structs.hpp"

void hitman2_silent_assassin::register_game_info(
    std::vector<GameInfo>& registry, const settings::Gui& settings
) {
    registry.emplace_back(
        GameInfo{
            .name = GAME_NAME,
            .methods
            = GameMethods{gui(settings, "Steam"), hook_nothing, hook_immediately_ready, update_slow(Version::Steam), update_fast(Version::Steam)},
            .make_remote_state = [] { return std::make_any<structs::Game>(); },
            .make_stats = [] { return std::make_any<hitman_common::Stats>(); },
            .module_infos = {{"hitman2.exe", PeId{0x3EF859D5}}},
        }
    );
};
