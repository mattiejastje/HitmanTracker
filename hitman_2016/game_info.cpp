#include "game_info.hpp"

#include "gui.hpp"

void hitman_2016::register_game_info(
    std::vector<GameInfo>& registry, const settings::Gui& settings
) {
    struct VersionSpec {
        const char* tag_suffix;
        const char* version_display;
        PeId pe_id;
    };

    constexpr VersionSpec specs[] = {
        // Direct3D 11: retail/HITMAN.exe
        {"-s-dx11", "Steam DX11", PeId{0x5F8ED8B9}},
        // Direct3D 12: dx12Retail/HITMAN.exe
        {"-s-dx12", "Steam DX12", PeId{0x5F8ED8D0}},
    };
    for (const auto& spec : specs) {
        GameMethods methods{
            gui(settings, spec.version_display),
            hook_nothing,
            hook_immediately_ready,
            stats_nothing_slow,
            stats_nothing_fast,
        };
        registry.emplace_back(
            GameInfo{
                .tag = std::string("h1") + spec.tag_suffix,
                .methods = std::move(methods),
                .make_remote_state = [] { return std::any{}; },
                .make_stats = [] { return std::any{}; },
                .module_infos = {{"hitman.exe", spec.pe_id}},
            }
        );
    }
}
