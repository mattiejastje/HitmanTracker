#include "gui.hpp"

#include <imgui.h>

#include <array>
#include <ranges>
#include <string>
#include <vector>

#include "../color.hpp"
#include "../format_duration.hpp"
#include "../imgui_app/key_value_table.hpp"
#include "../imgui_app/text.hpp"

std::vector<imgui_app::FontSpec>
hitman_common::make_font_specs(const settings::Gui& settings) {
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
    imgui_app::text(
        fonts[FontIndex::Title], im_vec4(settings.title.color), title.c_str()
    );
    imgui_app::text(
        fonts[FontIndex::Difficulty],
        im_vec4(settings.difficulty.color),
        difficulty.c_str()
    );
    if (map > 0) {
        ImGui::PushTextWrapPos();
        imgui_app::text(
            fonts[FontIndex::Map], im_vec4(settings.map.color), map_name.c_str()
        );
        ImGui::PopTextWrapPos();
        if (map_stage != MapStage::pre) {
            ImGui::Spacing();
            imgui_app::text(
                fonts[FontIndex::Time],
                im_vec4(settings.time.color),
                format_duration(time).c_str()
            );
        }
    }
    ImGui::Spacing();
}

static ImFont* rating_font(
    std::span<ImFont*> fonts, std::optional<Status> status
) {
    return status == Status::RED ? fonts[hitman_common::FontIndex::RatingBad]
           : status == Status::GREEN
               ? fonts[hitman_common::FontIndex::RatingGood]
               : fonts[hitman_common::FontIndex::RatingMaybe];
}

static ImVec4 rating_color(
    const settings::Gui& settings, std::optional<Status> status
) {
    return status == Status::GREEN ? im_vec4(settings.rating_good.color)
           : status == Status::RED ? im_vec4(settings.rating_bad.color)
                                   : im_vec4(settings.rating_maybe.color);
}

void hitman_common::gui_table(
    const settings::Gui& settings,
    std::span<ImFont*> fonts,
    const char* table_id,
    const StatsValue<std::string>& rating,
    int map,
    MapStage map_stage,
    const std::vector<TableRow>& table_rows
) {
    if (map <= 0 || map_stage == MapStage::pre) return;
    imgui_app::Header header{
        rating_font(fonts, rating.status),
        rating_color(settings, rating.status),
        rating.value,
    };
    auto to_entry = [&settings](const TableRow& row) -> imgui_app::TableEntry {
        if (std::holds_alternative<std::monostate>(row.value)) {
            return imgui_app::Spacer{};
        }
        auto* val_int = std::get_if<StatsValue<int32_t>>(&row.value);
        auto* val_str = std::get_if<StatsValue<std::string>>(&row.value);
        assert(val_int || val_str);
        std::string value_text
            = val_int ? std::format("{}", val_int->value) : val_str->value;
        std::optional<Status> status
            = val_int ? val_int->status : val_str->status;
        return imgui_app::Row{
            row.name,
            value_text,
            status ? rating_color(settings, status)
                   : im_vec4(settings.value.color),
        };
    };
    auto entries = table_rows | std::views::transform(to_entry)
                   | std::ranges::to<std::vector>();
    imgui_app::draw_key_value_table(
        table_id,
        header,
        fonts[FontIndex::Label],
        im_vec4(settings.label.color),
        fonts[FontIndex::Value],
        entries
    );
}
