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
    register_game_variants<VersionSpec>(
        registry,
        "h1",
        specs,
        [&settings](const VersionSpec& spec) {
            return GameMethods{
                gui(settings, spec.version_display),
                hook_nothing,
                hook_immediately_ready,
                stats_nothing_slow,
                stats_nothing_fast,
            };
        },
        [](const VersionSpec& spec) {
            return std::vector<ModuleInfo>{{"hitman.exe", spec.pe_id}};
        },
        [] { return std::any{}; },
        [] { return std::any{}; }
    );
}
