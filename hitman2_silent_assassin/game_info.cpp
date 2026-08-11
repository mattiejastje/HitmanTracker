#include "game_info.hpp"

#include "../hitman_common/stats.hpp"
#include "gui.hpp"
#include "stats.hpp"
#include "structs.hpp"

void hitman2_silent_assassin::register_game_info(
    std::vector<GameInfo>& registry, const settings::Gui& settings
) {
    struct VersionSpec {
        Version version;
        const char* tag_suffix;
        const char* version_display;
        PeId pe_id;
    };

    constexpr VersionSpec specs[] = {
        {Version::Steam, "-s", "Steam v1.02", PeId{0x3EF859D5}},
        // time date stamp not set on GOG version
        {Version::GOG, "-g", "GOG v1.01", PeId{0x0}},
    };
    register_game_variants<VersionSpec>(
        registry,
        "h2sa",
        specs,
        [&settings](const VersionSpec& spec) {
            return GameMethods{
                gui(settings, spec.version_display),
                hook_nothing,
                hook_immediately_ready,
                update_slow(spec.version),
                update_fast(spec.version),
            };
        },
        [](const VersionSpec& spec) {
            return std::vector<ModuleInfo>{{"hitman2.exe", spec.pe_id}};
        },
        [] { return std::make_any<structs::Game>(); },
        [] { return std::make_any<hitman_common::Stats>(); }
    );
}
