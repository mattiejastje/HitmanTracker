#include "gui.hpp"

#include <imgui.h>

#include "../hitman_common/gui.hpp"

void hitman_contracts::gui(
    const settings::Gui& settings, const Fonts& fonts, const Stats& stats
) {
    hitman_common::gui(
        settings, fonts, stats, "Hitman: Codename 47", "", {}, {}
    );
}
