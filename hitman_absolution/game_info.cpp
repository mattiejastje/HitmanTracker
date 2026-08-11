#include "game_info.hpp"

#include "gui.hpp"
#include "hook.hpp"
#include "stats.hpp"
#include "structs.hpp"

void hitman_absolution::register_game_info(
    std::vector<GameInfo>& registry,
    const settings::Gui& settings,
    const settings::HMA& hma
) {
    struct VersionSpec {
        Version version;
        const char* tag_suffix;
        const char* version_display;
        PeId pe_id;
    };

    constexpr VersionSpec specs[] = {
        {Version::Steam, "-s", "Steam", PeId{0x5149E0B4}},
        {Version::GOG, "-g", "GOG", PeId{0x5C9A0BF7}},
    };
    register_game_variants<VersionSpec>(
        registry,
        "hma",
        specs,
        [&settings, &hma](const VersionSpec& spec) {
            return GameMethods{
                gui(settings, hma, spec.version_display),
                hook(spec.version),
                hook_ready(spec.version),
                update_slow(hma, spec.version),
                update_fast(spec.version),
            };
        },
        [](const VersionSpec& spec) {
            return std::vector<ModuleInfo>{{"hma.exe", spec.pe_id}};
        },
        [] { return std::make_any<structs::Game>(); },
        [] { return std::make_any<Stats>(); }
    );
}
