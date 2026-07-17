#include "gui.hpp"

#include <imgui.h>

#include <format>

#include "../hitman_common/gui.hpp"
#include "stats.hpp"

const std::vector<std::string> map_names = {
    "",
    "#1 Training",
    "#2 Kowloon Triads in Gang War",
    "#3 Ambush at the Wang Fou Restaurant",
    "#4 The Massacre at Cheung Chau Fish Restaurant",
    "#5 The Lee Hong Assassination",
    "#6 Find the U'wa Tribe",
    "#7 The Jungle God",
    "#8 Say Hello to my Little Friend",
    "#9 Traditions of the Trade",
    "#10 Gunrunner's Paradise",
    "#11 Plutonium Runs Loose",
    "#12 The Setup",
    "#13 Meet Your Brother",
};

GameGui hitman_codename_47::gui(
    const settings::Gui& settings, const std::string& version
) {
    return [&settings, version](std::span<ImFont*> fonts, const std::any& stats_any) {
        const auto& stats = std::any_cast<const Stats&>(stats_any);
        hitman_common::gui_header(
            settings,
            fonts,
            "Hitman: Codename 47",
            version,
            stats.difficulty == 0   ? "Easy"
            : stats.difficulty == 1 ? "Normal"
            : stats.difficulty == 2 ? "Hard"
                                    : "",
            map_names[stats.map],
            stats.map,
            stats.map_stage,
            stats.time
        );
    };
}
