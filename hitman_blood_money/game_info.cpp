#include "game_info.hpp"

#include "gui.hpp"
#include "hook.hpp"
#include "stats.hpp"
#include "structs.hpp"

void hitman_blood_money::register_game_info(
    std::vector<GameInfo>& registry,
    const settings::Gui& settings,
    const settings::HBM& hbm
) {
    struct VersionSpec {
        Version version;
        const char* tag_suffix;
        const char* version_display;
        PeId pe_id;
    };

    constexpr VersionSpec specs[] = {
        {Version::Steam, "-s", "Steam", PeId{0x447EF98A}},
        {Version::GOG, "-g", "GOG", PeId{0x4492B845}},
    };
    for (const auto& spec : specs) {
        GameMethods methods{
            gui(settings, hbm, spec.version_display),
            hook(spec.version),
            hook_immediately_ready,
            update_slow(spec.version),
            update_fast(spec.version),
        };
        registry.emplace_back(
            GameInfo{
                .tag = std::string("hbm") + spec.tag_suffix,
                .methods = methods,
                .make_remote_state
                = [] { return std::make_any<structs::Game>(); },
                .make_stats = [] { return std::make_any<Stats>(); },
                .module_infos = {{"hitmanbloodmoney.exe", spec.pe_id}},
            }
        );
    }
}
