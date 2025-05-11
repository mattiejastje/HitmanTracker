#pragma once

#include "../game.hpp"

namespace hitman_common {

struct TableRow {
    std::string name;
    StatsValue stats_value;
};

void gui(
    const settings::Gui& settings,
    const Fonts& fonts,
    const Stats& stats,
    const std::string& game_name,
    const std::vector<std::string>& map_names,
    const std::vector<TableRow>& table_rows
);

}  // namespace hitman_common