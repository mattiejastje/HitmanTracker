#include "gui.hpp"

#include <imgui.h>

#include <array>
#include <string>
#include <vector>

#include "../format_duration.hpp"
#include "../imgui_utils.hpp"

void hitman_common::gui_header(
    const settings::Gui& settings,
    const Fonts& fonts,
    const std::string& game_name,
    const std::string& version,
    const std::string& difficulty,
    const std::string& map_name,
    int map,
    MapStage map_stage,
    float time
) {
    const auto title = settings.show_game_version
                           ? std::format("{} [{}]", game_name, version)
                           : game_name;
    text(fonts.title, settings.title.color, title.c_str());
    text(fonts.difficulty, settings.difficulty.color, difficulty.c_str());
    if (map > 0) {
        ImGui::PushTextWrapPos();
        text(fonts.map, settings.map.color, map_name.c_str());
        ImGui::PopTextWrapPos();
        if (map_stage != MapStage::pre) {
            ImGui::Spacing();
            text(
                fonts.time, settings.time.color, format_duration(time).c_str()
            );
        }
    }
    ImGui::Spacing();
}

void hitman_common::gui_table(
    const settings::Gui& settings,
    const Fonts& fonts,
    const StatsValue<std::string>& rating,
    int map,
    MapStage map_stage,
    const std::vector<TableRow>& table_rows
) {
    if (map > 0 && map_stage != MapStage::pre) {
        auto rating_font = rating.status == Status::RED ? fonts.rating_bad
                           : rating.status == Status::GREEN
                               ? fonts.rating_good
                               : fonts.rating_maybe;
        auto rating_color
            = rating.status == Status::RED     ? settings.rating_bad.color
              : rating.status == Status::GREEN ? settings.rating_good.color
                                               : settings.rating_maybe.color;
        auto& rating_text = rating.value;
        text(rating_font, rating_color, rating_text.c_str());
        ImGui::Spacing();
        ImGui::BeginTable(
            "Statistics",
            2,
            ImGuiTableFlags_SizingFixedFit
                | ImGuiTableFlags_NoKeepColumnsVisible
                | ImGuiTableFlags_NoHostExtendX
        );
        for (auto& row : table_rows) {
            std::visit(
                [&](auto&& val) {
                    using T = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<T, std::monostate>) {
                        // empty row, just for spacing
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::TableNextColumn();
                    } else if constexpr (std::is_same_v<
                                             T,
                                             StatsValue<std::string>>) {
                        table_row(
                            fonts,
                            settings,
                            val.status,
                            row.name.c_str(),
                            val.value.c_str()
                        );
                    } else {
                        table_row(
                            fonts,
                            settings,
                            val.status,
                            row.name.c_str(),
                            "%d",
                            val.value
                        );
                    }
                },
                row.value
            );
        }
        ImGui::EndTable();
        ImGui::Spacing();
    }
}
