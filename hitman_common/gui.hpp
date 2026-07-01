#pragma once

#include <variant>

#include "../game.hpp"

namespace hitman_common {

struct TableRow {
    std::string name;
    std::variant<
        std::monostate,
        StatsValue<int32_t>,
        StatsValue<std::string>>
        value;
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