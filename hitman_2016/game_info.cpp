#include "game_info.hpp"

#include "gui.hpp"

void hitman_2016::register_game_info(
    std::vector<GameInfo>& registry, const settings::Gui& settings
) {
    registry.emplace_back(
        // Direct3D 11: retail/HITMAN.exe
        GameInfo{
            .tag = "h1-s-dx11",
            .methods = GameMethods{
                gui(settings, "Steam DX11"),
                hook_nothing,
                hook_immediately_ready,
                stats_nothing_slow,
                stats_nothing_fast,
            },
            .make_remote_state = [] { return std::any{}; },
            .make_stats = [] { return std::any{}; },
            .module_infos = {
                {"hitman.exe", PeId{0x5F8ED8B9}},
            },
        }
    );
    registry.emplace_back(
        // Direct3D 12: dx12Retail\HITMAN.exe
        GameInfo{
            .tag = "h1-s-dx12",
            .methods = GameMethods{
                gui(settings, "Steam DX12"),
                hook_nothing,
                hook_immediately_ready,
                stats_nothing_slow,
                stats_nothing_fast,
            },
            .make_remote_state = [] { return std::any{}; },
            .make_stats = [] { return std::any{}; },
            .module_infos = {
                {"hitman.exe", PeId{0x5F8ED8D0}},
            },
        }
    );
}
