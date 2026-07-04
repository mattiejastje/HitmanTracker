#include "gui.hpp"

#include <imgui.h>

#include <array>
#include <string>
#include <vector>

#include "../format_duration.hpp"
#include "../imgui_utils.hpp"

std::array<FontSpec, hitman_common::FontIndex::Count> hitman_common::make_font_specs(
    const settings::Gui& settings
) {
    return {{
        {settings.title.file, settings.font_size * settings.title.scale},
        {settings.difficulty.file,
         settings.font_size * settings.difficulty.scale},
        {settings.map.file, settings.font_size * settings.map.scale},
        {settings.time.file, settings.font_size * settings.time.scale},
        {settings.rating_bad.file,
         settings.font_size * settings.rating_bad.scale},
        {settings.rating_good.file,
         settings.font_size * settings.rating_good.scale},
        {settings.rating_maybe.file,
         settings.font_size * settings.rating_maybe.scale},
        {settings.label.file, settings.font_size * settings.label.scale},
        {settings.value.file, settings.font_size * settings.value.scale},
        {"fonts/proggyforever/ProggyForever-Regular.ttf", 12.0f},
    }};
}

void hitman_common::gui_header(
    const settings::Gui& settings,
    std::span<ImFont*> fonts,
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
    text(fonts[FontIndex::Title], settings.title.color, title.c_str());
    text(
        fonts[FontIndex::Difficulty],
        settings.difficulty.color,
        difficulty.c_str()
    );
    if (map > 0) {
        ImGui::PushTextWrapPos();
        text(fonts[FontIndex::Map], settings.map.color, map_name.c_str());
        ImGui::PopTextWrapPos();
        if (map_stage != MapStage::pre) {
            ImGui::Spacing();
            text(
                fonts[FontIndex::Time],
                settings.time.color,
                format_duration(time).c_str()
            );
        }
    }
    ImGui::Spacing();
}

void hitman_common::gui_table(
    const settings::Gui& settings,
    std::span<ImFont*> fonts,
    const StatsValue<std::string>& rating,
    int map,
    MapStage map_stage,
    const std::vector<TableRow>& table_rows
) {
    if (map > 0 && map_stage != MapStage::pre) {
        auto rating_font
            = rating.status == Status::RED     ? fonts[FontIndex::RatingBad]
              : rating.status == Status::GREEN ? fonts[FontIndex::RatingGood]
                                               : fonts[FontIndex::RatingMaybe];
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
