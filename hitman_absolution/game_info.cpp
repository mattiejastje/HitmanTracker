#include "game_info.hpp"

#include "../game_info.hpp"
#include "../game_info_registry.hpp"
#include "gui.hpp"
#include "hook.hpp"
#include "stats.hpp"
#include "structs.hpp"

void hitman_absolution::register_game_info(
    const settings::Gui& settings, const settings::HMA& hma
) {
    auto& registry = get_game_info_registry();
    registry.emplace_back(
        GameInfo{
            .name = GAME_NAME,
            .methods
            = GameMethods{gui(settings, hma, "Steam"), hook(Version::Steam), hook_ready(Version::Steam), update_slow(hma, Version::Steam), update_fast(Version::Steam)},
            .make_remote_state = [] { return std::make_any<structs::Game>(); },
            .make_stats = [] { return std::make_any<Stats>(); },
            .module_infos = {{"hma.exe", PeId{0x5149E0B4}}},
        }
    );
    registry.emplace_back(
        GameInfo{
            .name = GAME_NAME,
            .methods
            = GameMethods{gui(settings, hma, "GOG"), hook(Version::GOG), hook_ready(Version::GOG), update_slow(hma, Version::GOG), update_fast(Version::GOG)},
            .make_remote_state = [] { return std::make_any<structs::Game>(); },
            .make_stats = [] { return std::make_any<Stats>(); },
            .module_infos = {{"hma.exe", PeId{0x5C9A0BF7}}},
        }
    );
}
