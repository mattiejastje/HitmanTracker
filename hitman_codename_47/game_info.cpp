#include "game_info.hpp"

#include "../game_info.hpp"
#include "../game_info_registry.hpp"
#include "gui.hpp"
#include "stats.hpp"
#include "structs.hpp"

void hitman_codename_47::register_game_info(const settings::Gui& settings) {
    auto& registry = get_game_info_registry();
    registry.emplace_back(GameInfo {
        .name = GAME_NAME,
        .methods = GameMethods{
            gui(settings, "Steam"),
            hook_nothing,
            hook_immediately_ready,
            update_slow(Version::Steam),
            update_fast(Version::Steam),
        },
        .make_remote_state = [] { return std::make_any<structs::HitmanDlc>(); },
        .make_stats = [] { return std::make_any<Stats>(); },
        .module_infos = {
            {"hitman.exe", PeId{0x3A3E13DA}},
            {"hitmandlc.dlc", PeId{0x3A3E13D1}},
        },
    });
}
