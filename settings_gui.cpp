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
    const int max_index = IM_ARRAYSIZE(RATING_MODES) - 1;
    const int max_allowed_index = is_score_allowed ? max_index : 1;
    auto current = std::clamp(static_cast<int>(rating_mode), 0, max_index);
    bool result = false;
    if (ImGui::BeginCombo(text, RATING_MODES[current])) {
        for (int i = 0; i <= max_index; i++) {
            bool disabled = i > max_allowed_index;
            ImGui::BeginDisabled(disabled);
            bool selected = (i == current);
            if (ImGui::Selectable(RATING_MODES[i], selected)) {
                current = i;
                result = true;
            }
            ImGui::EndDisabled();
            if (disabled
                && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("Unrated checkpoints have no score");
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    mark_any(changed, result);
    if (result) rating_mode = static_cast<settings::HMA::RatingMode>(current);
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
            return ImGui::GetStyleColorVec4(ImGuiCol_Text);
    }
}

static bool has_pending_entries(spdlog::log_clock::time_point cleared_before) {
    auto recent = spdlog_ring_sink()->last_raw(1);
    return !recent.empty() && recent.back().time >= cleared_before;
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

static ImVec4 signal_color(const RawSignal& signal) {
    if (signal.err && signal.value > *signal.err) {
        return ImVec4{1.0f, 0.3f, 0.3f, 1.0f};
    }
    if (signal.warn && signal.value > *signal.warn) {
        return ImVec4{0.9f, 0.8f, 0.1f, 1.0f};
    }
    return ImGui::GetStyleColorVec4(ImGuiCol_Text);
}

static RawSignal make_fraction_signal(
    const char* name, float part, float whole
) {
    return RawSignal{
        name,
        "% of frame",
        FRACTION_WARN,
        FRACTION_ERROR,
        whole > 0.0f ? 100.0f * part / whole : 0.0f
    };
}

static void draw_timing_row(
    const char* label,
    const char* tooltip,
    const RawSignal& time_signal,
    const RawSignal* fraction_signal
) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(label);
    if (tooltip && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tooltip);
    ImGui::TableNextColumn();
    ImGui::Text("%.2f ms", time_signal.value * 1000.0f);
    ImGui::TableNextColumn();
    if (fraction_signal) {
        ImGui::TextColored(
            signal_color(*fraction_signal), "%.0f%%", fraction_signal->value
        );
    } else {
        ImGui::TextDisabled("-");
    }
}

static void draw_error_rate_row(
    const char* label, const char* tooltip, const RawSignal& signal
) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(label);
    if (tooltip && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tooltip);
    ImGui::TableNextColumn();
    ImGui::TextColored(signal_color(signal), "%.0f%%", signal.value);
}

static void draw_diagnostics_tab(const Diagnostics& diagnostics) {
    const auto slow_fraction = make_fraction_signal(
        "slow update fraction",
        diagnostics.slow_update.signal.value,
        diagnostics.frame_time.value
    );
    const auto fast_fraction = make_fraction_signal(
        "fast update fraction",
        diagnostics.fast_update.signal.value,
        diagnostics.frame_time.value
    );
    ImGui::SeparatorText("Tracker Performance");
    if (ImGui::BeginTable(
            "timing",
            3,
            ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV
                | ImGuiTableFlags_SizingFixedFit
        )) {
        ImGui::TableSetupColumn("Metric");
        ImGui::TableSetupColumn("Time");
        ImGui::TableSetupColumn("% of frame");
        ImGui::TableHeadersRow();
        draw_timing_row(
            "Frame time",
            "The tracker's frame time (not the game's).",
            diagnostics.frame_time,
            nullptr
        );
        draw_timing_row(
            "Slow update",
            "Time spent reading and processing stats (runs ~10x/s). ",
            diagnostics.slow_update.signal,
            &slow_fraction
        );
        draw_timing_row(
            "Fast update",
            "Time spent updating the timer (runs every frame).",
            diagnostics.fast_update.signal,
            &fast_fraction
        );
        ImGui::EndTable();
    }
    ImGui::SeparatorText("Memory Read Reliability");
    if (ImGui::BeginTable(
            "reliability",
            2,
            ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV
                | ImGuiTableFlags_SizingFixedFit
        )) {
        ImGui::TableSetupColumn("Metric");
        ImGui::TableSetupColumn("Failure rate");
        ImGui::TableHeadersRow();
        // inv_tau = 1.0 so roughly average over last second
        draw_error_rate_row(
            "Slow update",
            "Percentage of stats reads that failed in the last second.",
            diagnostics.slow_update_error_rate
        );
        draw_error_rate_row(
            "Fast update",
            "Percentage of timer reads that failed in the last second.",
            diagnostics.fast_update_error_rate
        );
        ImGui::EndTable();
    }
    ImGui::TextDisabled(
        "Occasional failures are normal (e.g. during level loads)."
    );
}

static SettingsChanged draw_logging_tab(
    settings::Log& settings, spdlog::log_clock::time_point& cleared_before
) {
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
    const auto pending = get_pending_entries(cleared_before);
    const auto count = pending.size();
    ImGui::Text(
        count > 0 ? "%d error(s) since last cleared"
                  : "No errors since last cleared",
        count
    );
    ImGui::BeginDisabled(count == 0);
    if (ImGui::Button("Copy to Clipboard")) {
        std::string clipboard_text;
        for (auto& entry : pending)
            clipboard_text += spdlog_format_entry(entry);
        ImGui::SetClipboardText(clipboard_text.c_str());
    }
    ImGui::SetItemTooltip(
        count > 0 ? "Copy %d error(s) to clipboard" : "No errors to copy", count
    );
    ImGui::SameLine();
    if (ImGui::Button("Clear###ClearRecentErrors")) {
        cleared_before = spdlog::log_clock::now();
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
                for (auto& entry : pending | std::views::reverse) {
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

SettingsChanged settings_gui(
    settings::Settings& settings, const Diagnostics& diagnostics
) {
    static spdlog::log_clock::time_point cleared_before{};
    SettingsChanged changed;
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
            mark_any(
                changed,
                ImGui::Checkbox(
                    "Merge innocents / enemies / police",
                    &settings.hbm.merge_npcs
                )
            );
            mark_any(
                changed,
                ImGui::Checkbox(
                    "Merge frisk failed / cover blown",
                    &settings.hbm.merge_frisk_cover
                )
            );
            mark_any(
                changed,
                ImGui::Checkbox(
                    "Merge bodies found", &settings.hbm.merge_bodies_found
                )
            );
            mark_any(
                changed,
                ImGui::Checkbox(
                    "Merge witnesses / on camera",
                    &settings.hbm.merge_witnesses_camera
                )
            );
            mark_any(
                changed,
                ImGui::Checkbox(
                    "Merge suit / custom weapons left",
                    &settings.hbm.merge_items_left
                )
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
        if (ImGui::BeginTabItem("Diagnostics")) {
            draw_diagnostics_tab(diagnostics);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(
                has_pending_entries(cleared_before) ? "Logging (!)###Logging"
                                                    : "Logging###Logging"
            )) {
            changed |= draw_logging_tab(settings.log, cleared_before);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    return changed;
}