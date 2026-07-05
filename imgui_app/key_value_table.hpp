#pragma once

#include <imgui.h>

#include <optional>
#include <span>
#include <string>
#include <variant>

namespace imgui_app {

struct Row {
    std::string label;
    std::string value;
    ImVec4 value_color;
};

struct Spacer {};

struct Header {
    ImFont* font;
    ImVec4 color;
    std::string text;
};

using TableEntry = std::variant<Row, Spacer>;

void draw_key_value_table(
    const char* table_id,
    Header header,
    ImFont* label_font,
    const ImVec4& label_color,
    ImFont* value_font,
    std::span<const TableEntry> rows
);

}  // namespace imgui_app