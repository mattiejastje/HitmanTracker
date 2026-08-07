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
        const char* display;
        PeId pe_id;
    };

    constexpr VersionSpec specs[] = {
        {Version::Steam, "-s", "Steam", PeId{0x5149E0B4}},
        {Version::GOG, "-g", "GOG", PeId{0x5C9A0BF7}},
    };
    for (auto& spec : specs) {
        registry.emplace_back(
            GameInfo{
                .tag = std::string("hma") + spec.tag_suffix,
                .methods
                = GameMethods{gui(settings, hma, spec.display), hook(spec.version), hook_ready(spec.version), update_slow(hma, spec.version), update_fast(spec.version)},
                .make_remote_state
                = [] { return std::make_any<structs::Game>(); },
                .make_stats = [] { return std::make_any<Stats>(); },
                .module_infos = {{"hma.exe", spec.pe_id}},
            }
        );
    }
}
