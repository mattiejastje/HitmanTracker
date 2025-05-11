#include "gui.hpp"

#include <imgui.h>

#include <array>
#include <string>
#include <vector>

#include "../format_duration.hpp"
#include "../imgui_utils.hpp"

void hitman_common::gui(
    const settings::Gui& settings,
    const Fonts& fonts,
    const Stats& stats,
    const std::string& game_name,
    const std::string& difficulty,
    const std::vector<std::string>& map_names,
    const std::vector<TableRow>& table_rows
) {
    text(fonts.title, settings.title.color, game_name.c_str());
    text(fonts.difficulty, settings.difficulty.color, difficulty.c_str());
    if (stats.map > 0) {
        text(
            fonts.map, settings.map.color, map_names.at(stats.map - 1).c_str()
        );
        if (stats.map_stage != MapStage::pre) {
            ImGui::Spacing();
            text(
                fonts.time,
                settings.time.color,
                format_duration(stats.time).c_str()
            );
            auto rating_font
                = stats.silent_assassin == Status::RED     ? fonts.rating_bad
                  : stats.silent_assassin == Status::GREEN ? fonts.rating_good
                                                           : fonts.rating_maybe;
            auto rating_color = stats.silent_assassin == Status::RED
                                    ? settings.rating_bad.color
                                : stats.silent_assassin == Status::GREEN
                                    ? settings.rating_good.color
                                    : settings.rating_maybe.color;
            auto rating_text = stats.silent_assassin == Status::GREEN
                                   ? "Silent Assassin"
                                   : "No Silent Assassin";
            text(rating_font, rating_color, rating_text);
            ImGui::Spacing();
            ImGui::BeginTable(
                "Statistics",
                2,
                ImGuiTableFlags_SizingFixedFit
                    | ImGuiTableFlags_NoKeepColumnsVisible
                    | ImGuiTableFlags_NoHostExtendX
            );
            for (auto& row : table_rows) {
                table_row(
                    fonts,
                    settings,
                    row.stats_value.status,
                    row.name.c_str(),
                    "%d",
                    row.stats_value.value
                );
            }
            ImGui::EndTable();
        }
    }
}
