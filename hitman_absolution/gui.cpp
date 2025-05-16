#include "gui.hpp"

#include "../hitman_common/gui.hpp"

void hitman_absolution::gui(
    const settings::Gui& settings, const Fonts& fonts, const Stats& stats
) {
    hitman_common::gui(
        settings,
        fonts,
        stats,
        "Hitman: Absolution",
        "",
        {},
        {}
    );
}
