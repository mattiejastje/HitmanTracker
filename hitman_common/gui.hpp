#pragma once

#include <variant>

#include "../game.hpp"

namespace hitman_common {

struct Double {
    StatsValue<double> stats_value;
    const char* format = "%.3g";
};

struct TableRow {
    std::string name;
    std::variant<StatsValue<int32_t>, Double, std::monostate>
        stats_value;
};

void gui_header(
    const settings::Gui& settings,
    const Fonts& fonts,
    const std::string& game_name,
    const std::string& version,
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