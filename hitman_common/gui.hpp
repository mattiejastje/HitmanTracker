#pragma once

#include <span>
#include <variant>

#include "../color.hpp"
#include "../game.hpp"
#include "../imgui_app/text.hpp"
#include "../settings.hpp"

namespace hitman_common {

enum FontIndex {
    Title,
    Difficulty,
    Map,
    Time,
    RatingBad,
    RatingGood,
    RatingMaybe,
    Label,
    Value,
    Settings,
    Count
};

std::vector<imgui_app::FontSpec> make_font_specs(const settings::Gui& settings);

struct TableRow {
    std::string name;
    std::variant<std::monostate, StatsValue<int32_t>, StatsValue<std::string>>
        value;
};

void gui_header(
    const settings::Gui& settings,
    std::span<ImFont*> fonts,
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
    std::span<ImFont*> fonts,
    const char* table_id,
    const StatsValue<std::string>& rating,
    int map,
    MapStage map_stage,
    const std::vector<TableRow>& table_rows
);

}  // namespace hitman_common