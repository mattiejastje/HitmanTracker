#pragma once

#include "../game.hpp"

namespace hitman_common {

template <typename T>
struct TableRow {
    std::string name;
    StatsValue<T> stats_value;
};

void gui(
    const settings::Gui& settings,
    const Fonts& fonts,
    const Stats& stats,
    const std::string& game_name,
    const std::string& difficulty,
    const std::vector<std::string>& map_names,
    const std::vector<TableRow<int32_t>>& table_rows
);

}  // namespace hitman_common