#include "settings_gui.hpp"

#include <algorithm>
#include <ranges>

#include "imgui_app/modal_popup.hpp"
#include "shell.hpp"
#include "spdlog.hpp"

SettingsChanged& operator|=(SettingsChanged& a, const SettingsChanged& b) {
    a.any |= b.any;
    a.fonts |= b.fonts;
    a.overlay_mode |= b.overlay_mode;
    a.reposition |= b.reposition;
    a.logging |= b.logging;
    return a;
}

static void mark_any(SettingsChanged& changed, bool v) { changed.any |= v; }

static void mark_fonts(SettingsChanged& changed, bool v) {
    changed.any |= v;
    changed.fonts |= v;
}

static void mark_overlay_mode(SettingsChanged& changed, bool v) {
    changed.any |= v;
    changed.overlay_mode |= v;
}

static void mark_logging(SettingsChanged& changed, bool v) {
    changed.any |= v;
    changed.logging |= v;
}

static const char* RATING_MODES[]
    = {"None", "SA", "Score", "SA & Score", "SA with Score fallback"};

static bool combo_rating_mode(
    SettingsChanged& changed,
    const char* text,
    settings::HMA::RatingMode& rating_mode,
    bool is_score_allowed
) {
    const int max_index
        = (is_score_allowed ? IM_ARRAYSIZE(RATING_MODES) : 2) - 1;
    auto rating_mode_int
        = std::clamp(static_cast<int>(rating_mode), 0, max_index);
    ImGui::Combo(text, &rating_mode_int, RATING_MODES, max_index + 1);
    bool result = rating_mode_int != static_cast<int>(rating_mode);
    mark_any(changed, result);
    rating_mode = static_cast<settings::HMA::RatingMode>(rating_mode_int);
    return result;
}

static bool slider_float(
    const char* label, float* v, float lo, float hi, float step, const char* fmt
) {
    float old = *v;
    if (ImGui::SliderFloat(label, v, lo, hi, fmt)) {
        *v = step * std::round(*v / step);
    }
    return *v != old;
}

static void text_style_gui(
    const char* label, settings::TextStyle& style, SettingsChanged& changed
) {
    ImGui::PushID(label);
    if (ImGui::TreeNode(label)) {
        mark_fonts(
            changed,
            slider_float("Scale", &style.scale, 0.5f, 2.0f, 0.1f, "%.1f")
        );
        mark_any(changed, ImGui::ColorEdit3("Color", style.color.data()));
        ImGui::TreePop();
    }
    ImGui::PopID();
}

static ImVec4 level_color(spdlog::level::level_enum lvl) {
    switch (lvl) {
        case spdlog::level::trace:
            return {0.6f, 0.6f, 0.6f, 1.0f};
        case spdlog::level::debug:
            return {0.5f, 0.5f, 0.9f, 1.0f};
        case spdlog::level::warn:
            return {0.9f, 0.8f, 0.1f, 1.0f};
        case spdlog::level::err:
            return {1.0f, 0.3f, 0.3f, 1.0f};
        case spdlog::level::critical:
            return {1.0f, 0.1f, 0.6f, 1.0f};
        default:
            return {1.0f, 1.0f, 1.0f, 1.0f};
    }
}

static std::vector<spdlog::details::log_msg_buffer> get_pending_entries(
    spdlog::log_clock::time_point cleared_before
) {
    auto entries = spdlog_ring_sink()->last_raw();
    std::erase_if(entries, [cleared_before](const auto& e) {
        return e.time < cleared_before;
    });
    return entries;
}

static SettingsChanged draw_logging_tab(settings::Log& settings) {
    static spdlog::log_clock::time_point cleared_before{};
    SettingsChanged changed{};
    ImGui::SeparatorText("Log Files");
    if (ImGui::Button("Open Folder"))
        shell_open_file(spdlog_log_dir().wstring().c_str());
    mark_logging(
        changed,
        ImGui::Checkbox(
            "Include trace-level detail (slower)", &settings.capture_trace
        )
    );
    ImGui::SetItemTooltip(
        "Captures the most verbose detail into the logs "
        "(has a significant performance cost)"
    );
    ImGui::SeparatorText("Recent Errors");
    auto counter_sink = spdlog_counter_sink();
    const auto count = counter_sink ? counter_sink->count.load() : 0;
    ImGui::Text(
        count > 0 ? "%d error(s) since last cleared"
                  : "No errors since last cleared",
        count
    );
    ImGui::BeginDisabled(count == 0);
    if (ImGui::Button("Copy to Clipboard")) {
        std::string clipboard_text;
        for (auto& entry : get_pending_entries(cleared_before))
            clipboard_text += spdlog_format_entry(entry);
        ImGui::SetClipboardText(clipboard_text.c_str());
    }
    ImGui::SetItemTooltip(
        count > 0 ? "Copy %d error(s) to clipboard" : "No errors to copy", count
    );
    ImGui::SameLine();
    if (ImGui::Button("Clear###ClearRecentErrors")) {
        cleared_before = spdlog::log_clock::now();
        if (counter_sink) counter_sink->count = 0;
    }
    ImGui::SetItemTooltip(
        count > 0 ? "Clear %d error(s)" : "No errors to clear", count
    );
    ImGui::EndDisabled();
    changed.any
        |= ImGui::Checkbox("Show details", &settings.show_recent_errors);
    ImGui::BeginDisabled(count == 0);
    if (settings.show_recent_errors) {
        if (ImGui::BeginChild("log_table")) {
            if (ImGui::BeginTable(
                    "logs",
                    3,
                    ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY
                        | ImGuiTableFlags_SizingFixedFit
                )) {
                ImGui::TableSetupColumn("Time");
                ImGui::TableSetupColumn("Level");
                ImGui::TableSetupColumn("Message");
                ImGui::TableHeadersRow();
                for (auto& entry : get_pending_entries(cleared_before)
                                       | std::views::reverse) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(spdlog_format_time(entry).c_str());
                    ImGui::TableNextColumn();
                    auto lvl_name = spdlog::level::to_string_view(entry.level);
                    ImGui::TextColored(
                        level_color(entry.level),
                        "%.*s",
                        (int)lvl_name.size(),
                        lvl_name.data()
                    );
                    ImGui::TableNextColumn();
                    std::string payload(
                        entry.payload.data(), entry.payload.size()
                    );
                    ImGui::TextUnformatted(payload.c_str());
                }
            }
            ImGui::EndTable();
        }
        ImGui::EndChild();
    }
    ImGui::EndDisabled();
    return changed;
}

SettingsChanged settings_gui(settings::Settings& settings) {
    SettingsChanged changed;
    bool open_clear_log_files{false};
    if (ImGui::BeginTabBar("Settings")) {
        if (ImGui::BeginTabItem("General")) {
            mark_overlay_mode(
                changed,
                ImGui::Checkbox("Overlay mode", &settings.gui.overlay_mode)
            );
            ImGui::SetItemTooltip(
                "Make tracker transparent, click-through, and always on top"
            );
            mark_fonts(
                changed,
                slider_float(
                    "Font size",
                    &settings.gui.font_size,
                    8.0f,
                    64.0f,
                    1.0f,
                    "%.0f"
                )
            );
            ImGui::BeginDisabled(settings.gui.overlay_mode);
            mark_any(
                changed,
                ImGui::SliderInt(
                    "Border size", &settings.gui.border_size, 0, 10
                )
            );
            ImGui::SetItemTooltip("Border size in non-overlay mode");
            ImGui::EndDisabled();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Layout")) {
            mark_any(
                changed,
                ImGui::Checkbox("Show game name", &settings.gui.show_game)
            );
            mark_any(
                changed,
                ImGui::Checkbox("Show game version", &settings.gui.show_version)
            );
            mark_any(
                changed,
                ImGui::Checkbox(
                    "Show difficulty", &settings.gui.show_difficulty
                )
            );
            mark_any(
                changed, ImGui::Checkbox("Show map", &settings.gui.show_map)
            );
            mark_any(
                changed, ImGui::Checkbox("Show time", &settings.gui.show_time)
            );
            mark_any(
                changed,
                ImGui::Checkbox("Show rating", &settings.gui.show_rating)
            );
            mark_any(
                changed,
                ImGui::Checkbox("Show statistics", &settings.gui.show_stats)
            );
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Style")) {
            text_style_gui("Game", settings.gui.game, changed);
            text_style_gui("Version", settings.gui.version, changed);
            text_style_gui("Difficulty", settings.gui.difficulty, changed);
            text_style_gui("Map", settings.gui.map, changed);
            text_style_gui("Time", settings.gui.time, changed);
            ImGui::PushID("Rating");
            if (ImGui::TreeNode("Rating")) {
                mark_fonts(
                    changed,
                    slider_float(
                        "Scale",
                        &settings.gui.rating_bad.scale,
                        0.5f,
                        2.0f,
                        0.1f,
                        "%.1f"
                    )
                );
                settings.gui.rating_maybe.scale = settings.gui.rating_bad.scale;
                settings.gui.rating_good.scale = settings.gui.rating_bad.scale;
                mark_any(
                    changed,
                    ImGui::ColorEdit3(
                        "Bad", settings.gui.rating_bad.color.data()
                    )
                );
                mark_any(
                    changed,
                    ImGui::ColorEdit3(
                        "Maybe", settings.gui.rating_maybe.color.data()
                    )
                );
                mark_any(
                    changed,
                    ImGui::ColorEdit3(
                        "Good", settings.gui.rating_good.color.data()
                    )
                );
                ImGui::TreePop();
            }
            ImGui::PopID();
            ImGui::PushID("Statistics");
            if (ImGui::TreeNode("Statistics")) {
                mark_fonts(
                    changed,
                    slider_float(
                        "Scale",
                        &settings.gui.label.scale,
                        0.5f,
                        2.0f,
                        0.1f,
                        "%.1f"
                    )
                );
                settings.gui.value.scale = settings.gui.label.scale;
                mark_any(
                    changed,
                    ImGui::ColorEdit3("Value", settings.gui.value.color.data())
                );
                mark_any(
                    changed,
                    ImGui::ColorEdit3("Label", settings.gui.label.color.data())
                );
                ImGui::TreePop();
            }
            ImGui::PopID();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Blood Money")) {
            mark_any(
                changed,
                ImGui::Checkbox("Use real time", &settings.hbm.real_time)
            );
            mark_any(
                changed,
                ImGui::Checkbox(
                    "Show accident kills", &settings.hbm.show_accident_kills
                )
            );
            mark_any(
                changed,
                ImGui::Checkbox("Show shots hit", &settings.hbm.show_shots_hit)
            );
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Absolution")) {
            mark_any(
                changed,
                ImGui::Checkbox(
                    "Show SA rating details", &settings.hma.show_sa_details
                )
            );
            mark_any(
                changed,
                ImGui::Checkbox(
                    "Show max Score rating only",
                    &settings.hma.show_max_score_rating_only
                )
            );
            ImGui::SetItemTooltip("Only show Shadow / No Shadow etc.");
            mark_any(
                changed,
                ImGui::Checkbox(
                    "Show Score rating total", &settings.hma.show_score_total
                )
            );
            mark_any(
                changed,
                ImGui::Checkbox(
                    "Show Score rating details",
                    &settings.hma.show_score_details
                )
            );
            mark_any(
                changed,
                ImGui::Checkbox(
                    "Apply difficulty/challenge bonus",
                    &settings.hma.apply_bonus
                )
            );
            ImGui::PushID("Rating Mode");
            if (ImGui::TreeNode("Rating Mode")) {
                if (ImGui::Button("Original Game")) {
                    settings.hma.rating_mode_unrated
                        = settings::HMA::RatingMode::X;
                    settings.hma.rating_mode_no_targets
                        = settings::HMA::RatingMode::SC;
                    settings.hma.rating_mode_targets
                        = settings::HMA::RatingMode::SA_FALLBACK_SC;
                    changed.any |= true;
                };
                ImGui::SameLine();
                if (ImGui::Button("Max Rating")) {
                    settings.hma.rating_mode_unrated
                        = settings::HMA::RatingMode::X;
                    settings.hma.rating_mode_no_targets
                        = settings::HMA::RatingMode::SA_PLUS_SC;
                    settings.hma.rating_mode_targets
                        = settings::HMA::RatingMode::SA;
                    changed.any |= true;
                };
                ImGui::SameLine();
                if (ImGui::Button("SA")) {
                    settings.hma.rating_mode_unrated
                        = settings::HMA::RatingMode::SA;
                    settings.hma.rating_mode_no_targets
                        = settings::HMA::RatingMode::SA;
                    settings.hma.rating_mode_targets
                        = settings::HMA::RatingMode::SA;
                    changed.any |= true;
                };
                ImGui::SameLine();
                if (ImGui::Button("Full Tracking")) {
                    settings.hma.rating_mode_unrated
                        = settings::HMA::RatingMode::SA;
                    settings.hma.rating_mode_no_targets
                        = settings::HMA::RatingMode::SA_PLUS_SC;
                    settings.hma.rating_mode_targets
                        = settings::HMA::RatingMode::SA_PLUS_SC;
                    changed.any |= true;
                };
                combo_rating_mode(
                    changed,
                    "Unrated checkpoints",
                    settings.hma.rating_mode_unrated,
                    false
                );
                combo_rating_mode(
                    changed,
                    "Checkpoints without targets",
                    settings.hma.rating_mode_no_targets,
                    true
                );
                combo_rating_mode(
                    changed,
                    "Checkpoints with targets",
                    settings.hma.rating_mode_targets,
                    true
                );
                ImGui::TreePop();
            }
            ImGui::PopID();
            ImGui::EndTabItem();
        }
        auto counter_sink = spdlog_counter_sink();
        const auto count = counter_sink ? counter_sink->count.load() : 0;
        if (ImGui::BeginTabItem(
                count > 0 ? "Logging (!)###Logging" : "Logging###Logging"
            )) {
            changed |= draw_logging_tab(settings.log);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    return changed;
}