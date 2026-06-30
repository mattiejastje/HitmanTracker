#pragma once

#include "../game.hpp"

namespace hitman_common {

struct TableRow {
    std::string name;
    StatsValue<int32_t> stats_value;
};

void gui_header(
    const settings::Gui& settings,
    const Fonts& fonts,
    const std::string& game_name,
    const std::string& difficulty,
    const std::string& map_name,
    int map,
    MapStage map_stage,
    float time
);

void gui_table(
    const settings::Gui& settings,
    const Fonts& fonts,
    const StatsValue<std::string>& rating,
    int map,
    MapStage map_stage,
    const std::vector<TableRow>& table_rows
);

}  // namespace hitman_common