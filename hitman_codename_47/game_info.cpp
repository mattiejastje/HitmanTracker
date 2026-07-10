#include "game_info.hpp"

#include "gui.hpp"
#include "stats.hpp"
#include "structs.hpp"

void hitman_codename_47::register_game_info(
    std::vector<GameInfo>& registry, const settings::Gui& settings
) {
    registry.emplace_back(GameInfo {
        .name = GAME_NAME,
        .methods = GameMethods{
            gui(settings, "Steam b192"),
            hook_nothing,
            hook_immediately_ready,
            update_slow(Version::SteamOrGOG),
            update_fast(Version::SteamOrGOG),
        },
        .make_remote_state = [] { return std::make_any<structs::HitmanDlc>(); },
        .make_stats = [] { return std::make_any<Stats>(); },
        .module_infos = {
            {"hitman.exe", PeId{0x3A3E13DA}},
            {"hitmandlc.dlc", PeId{0x3A3E13D1}},
        },
    });
    registry.emplace_back(GameInfo {
        .name = GAME_NAME,
        .methods = GameMethods{
            gui(settings, "GOG b192"),
            hook_nothing,
            hook_immediately_ready,
            update_slow(Version::SteamOrGOG),
            update_fast(Version::SteamOrGOG),
        },
        .make_remote_state = [] { return std::make_any<structs::HitmanDlc>(); },
        .make_stats = [] { return std::make_any<Stats>(); },
        .module_infos = {
            {"hitman.exe", PeId{0x73655965}},
            {"hitmandlc.dlc", PeId{0x3A27B1FA}},
        },
    });
}
