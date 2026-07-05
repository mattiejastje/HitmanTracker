#include "key_value_table.hpp"

#include "text.hpp"

void imgui_app::draw_key_value_table(
    const char* table_id,
    Header header,
    ImFont* key_font,
    const ImVec4& key_color,
    ImFont* value_font,
    std::span<const TableEntry> entries
) {
    text(header.font, header.color, header.text.c_str());
    ImGui::Spacing();
    if (ImGui::BeginTable(
            table_id,
            2,
            ImGuiTableFlags_SizingFixedFit
                | ImGuiTableFlags_NoKeepColumnsVisible
                | ImGuiTableFlags_NoHostExtendX
        )) {
        for (auto& entry : entries) {
            const auto* row = std::get_if<Row>(&entry);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (row) text(value_font, row->value_color, row->value.c_str());
            ImGui::TableNextColumn();
            if (row) text(key_font, key_color, row->key.c_str());
        }
        ImGui::EndTable();
    }
    ImGui::Spacing();
}
