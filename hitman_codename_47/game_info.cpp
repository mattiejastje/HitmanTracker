#include "game_info.hpp"

#include "gui.hpp"
#include "stats.hpp"
#include "structs.hpp"

void hitman_codename_47::register_game_info(
    std::vector<GameInfo>& registry, const settings::Gui& settings
) {
    struct VersionSpec {
        const char* tag_suffix;
        const char* version_display;
        PeId pe_id_exe;
        PeId pe_id_dlc;
    };

    constexpr VersionSpec specs[] = {
        {"-s", "Steam b192", PeId{0x3A3E13DA}, PeId{0x3A3E13D1}},
        {"-g", "GOG b192", PeId{0x73655965}, PeId{0x3A27B1FA}},
    };
    for (const auto& spec : specs) {
        GameMethods methods{
            gui(settings, spec.version_display),
            hook_nothing,
            hook_immediately_ready,
            update_slow(Version::SteamOrGOG),
            update_fast(Version::SteamOrGOG),
        };
        std::vector<ModuleInfo> module_infos = {
            {"hitman.exe", spec.pe_id_exe},
            {"hitmandlc.dlc", spec.pe_id_dlc},
        };
        registry.emplace_back(
            GameInfo{
                .tag = std::string("hc47") + spec.tag_suffix,
                .methods = std::move(methods),
                .make_remote_state
                = [] { return std::make_any<structs::HitmanDlc>(); },
                .make_stats = [] { return std::make_any<Stats>(); },
                .module_infos = std::move(module_infos),
            }
        );
    }
}
