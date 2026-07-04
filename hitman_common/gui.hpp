#pragma once

#include <span>
#include <variant>

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

std::array<FontSpec, FontIndex::Count> make_font_specs(
    const settings::Gui& settings
);

struct TableRow {
    std::string name;
    std::variant<std::monostate, StatsValue<int32_t>, StatsValue<std::string>>
        value;
};

template <typename... Args>
void table_row(
    ImFont* label_font,
    std::array<float, 3> label_color,
    ImFont* value_font,
    std::array<float, 3> value_color,
    const char* label,
    const char* value,
    Args... args
) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    text(value_font, value_color, value, args...);
    ImGui::TableNextColumn();
    text(label_font, label_color, label);
}

template <typename... Args>
void table_row(
    std::span<ImFont*> fonts,
    const settings::Gui& settings,
    std::optional<Status> status,
    const char* label,
    const char* value,
    Args... args
) {
    auto value_color
        = !status.has_value()               ? settings.value.color
          : status.value() == Status::GREEN ? settings.rating_good.color
          : status.value() == Status::RED   ? settings.rating_bad.color
                                            : settings.rating_maybe.color;
    table_row(
        fonts[FontIndex::Label],
        settings.label.color,
        fonts[FontIndex::Value],
        value_color,
        label,
        value,
        args...
    );
}

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
    const StatsValue<std::string>& rating,
    int map,
    MapStage map_stage,
    const std::vector<TableRow>& table_rows
);

}  // namespace hitman_common