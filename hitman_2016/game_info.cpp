#include "game_info.hpp"

#include "../game_info.hpp"
#include "../game_info_registry.hpp"
#include "gui.hpp"

void hitman_2016::register_game_info(const settings::Gui& settings) {
    auto& registry = get_game_info_registry();
    registry.emplace_back(
        // Direct3D 11: retail/HITMAN.exe
        GameInfo{
            .name = GAME_NAME,
            .methods = GameMethods{
                gui(settings, "Steam DX11"),
                hook_nothing,
                hook_immediately_ready,
                stats_nothing_slow,
                stats_nothing_fast,
            },
            .module_infos = {
                {"hitman.exe", PeId{0x5F8ED8B9}},
            },
        }
    );
    registry.emplace_back(
        // Direct3D 12: dx12Retail\HITMAN.exe
        GameInfo{
            .name = GAME_NAME,
            .methods = GameMethods{
                gui(settings, "Steam DX12"),
                hook_nothing,
                hook_immediately_ready,
                stats_nothing_slow,
                stats_nothing_fast,
            },
            .module_infos = {
                {"hitman.exe", PeId{0x5F8ED8D0}},
            },
        }
    );
}
