#include "game_info.hpp"

#include "../hitman_common/stats.hpp"
#include "gui.hpp"
#include "stats.hpp"
#include "structs.hpp"

void hitman_contracts::register_game_info(
    std::vector<GameInfo>& registry, const settings::Gui& settings
) {
    struct VersionSpec {
        Version version;
        const char* tag_suffix;
        const char* version_display;
        PeId pe_id;
    };

    constexpr VersionSpec specs[] = {
        {Version::Steam, "-s", "Steam Build 175", PeId{0x52B2D458}},
        {Version::GOG, "-g", "GOG Build 174", PeId{0x4091141B}},
    };
    register_game_variants<VersionSpec>(
        registry,
        "hcon",
        specs,
        [&settings](const VersionSpec& spec) {
            return GameMethods{
                gui(settings, spec.version_display),
                hook_nothing,
                hook_immediately_ready,
                update_slow(spec.version),
                update_fast(spec.version)
            };
        },
        [](const VersionSpec& spec) {
            return std::vector<ModuleInfo>{{"hitmancontracts.exe", spec.pe_id}};
        },
        [] { return std::make_any<structs::HitmanContracts>(); },
        [] { return std::make_any<hitman_common::Stats>(); }
    );
}
