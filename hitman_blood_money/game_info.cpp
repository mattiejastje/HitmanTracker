#include "game_info.hpp"

#include "gui.hpp"
#include "hook.hpp"
#include "stats.hpp"
#include "structs.hpp"

void hitman_blood_money::register_game_info(
    std::vector<GameInfo>& registry,
    const settings::Gui& settings,
    const settings::HBM& hbm
) {
    registry.emplace_back(
        GameInfo{
        .name = GAME_NAME,
        .methods = GameMethods{
            gui(settings, hbm, "Steam"),
            hook,
            hook_immediately_ready,
            update_slow(Version::Steam),
            update_fast(Version::Steam),
        },
        .make_remote_state = [] { return std::make_any<structs::Game>(); },
        .make_stats = [] { return std::make_any<Stats>(); },
        .module_infos = {{"hitmanbloodmoney.exe", PeId{0x447EF98A}}},
    });
}
