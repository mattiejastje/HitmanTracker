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
    for (const auto& spec : specs) {
        GameMethods methods{
            gui(settings, spec.version_display),
            hook_nothing,
            hook_immediately_ready,
            update_slow(spec.version),
            update_fast(spec.version)
        };
        registry.emplace_back(
            GameInfo{
                .tag = std::string("hcon") + spec.tag_suffix,
                .methods = std::move(methods),
                .make_remote_state
                = [] { return std::make_any<structs::HitmanContracts>(); },
                .make_stats
                = [] { return std::make_any<hitman_common::Stats>(); },
                .module_infos = {{"hitmancontracts.exe", spec.pe_id}},
            }
        );
    }
}
