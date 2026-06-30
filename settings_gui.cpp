#include "settings_gui.hpp"

#include <imgui.h>

#include "spdlog.hpp"

static void mark_any(SettingsChanged& changed, bool v) { changed.any |= v; }

static void mark_fonts(SettingsChanged& changed, bool v) {
    changed.any |= v;
    changed.fonts |= v;
}

static void mark_topmost(SettingsChanged& changed, bool v) {
    changed.any |= v;
    changed.topmost |= v;
}

static const char* LOG_LEVEL_NAMES[]
    = {"off", "critical", "error", "warn", "info", "debug", "trace"};

static const char* RATING_MODES[]
    = {"None", "SA", "Score", "SA & Score", "SA with Score fallback"};

static bool combo_rating_mode(
    SettingsChanged& changed,
    const char* text,
    settings::HMA::RatingMode& rating_mode,
    bool is_score_allowed
) {
    auto rating_mode_int = static_cast<int>(rating_mode);
    bool result = ImGui::Combo(
        text,
        &rating_mode_int,
        RATING_MODES,
        is_score_allowed ? IM_ARRAYSIZE(RATING_MODES) : 2
    );
    mark_any(changed, result);
    rating_mode = static_cast<settings::HMA::RatingMode>(rating_mode_int);
    return result;
}

static bool slider_float(
    const char* label, float* v, float lo, float hi, float step
) {
    int lo_int = std::lround(lo / step);
    int hi_int = std::lround(hi / step);
    int v_int = std::lround((*v) / step);
    bool changed = ImGui::SliderInt(label, &v_int, lo_int, hi_int);
    *v = v_int * step;
    return changed;
}

static void text_style_gui(
    const char* label, settings::TextStyle& style, SettingsChanged& changed
) {
    ImGui::PushID(label);
    if (ImGui::TreeNode(label)) {
        mark_fonts(
            changed, slider_float("Scale", &style.scale, 0.5f, 2.0f, 0.1f)
        );
        mark_any(changed, ImGui::ColorEdit3("Color", style.color.data()));
        ImGui::TreePop();
    }
    ImGui::PopID();
}

SettingsChanged settings_gui(settings::Settings& settings) {
    SettingsChanged changed;
    if (ImGui::CollapsingHeader("User Interface")) {
        mark_topmost(
            changed,
            ImGui::Checkbox("Force topmost window", &settings.gui.topmost)
        );
        mark_any(
            changed,
            ImGui::Checkbox(
                "Show game version", &settings.gui.show_game_version
            )
        );
        mark_fonts(
            changed,
            slider_float(
                "Font size", &settings.gui.font_size, 8.0f, 64.0f, 1.0f
            )
        );
        // background needs ui refresh too
        mark_fonts(
            changed,
            ImGui::ColorEdit3("Background", settings.gui.bg_color.data())
        );
        text_style_gui("Game", settings.gui.title, changed);
        text_style_gui("Difficulty", settings.gui.difficulty, changed);
        text_style_gui("Map", settings.gui.map, changed);
        text_style_gui("Time", settings.gui.time, changed);
        ImGui::PushID("Rating");
        if (ImGui::TreeNode("Rating")) {
            mark_fonts(
                changed,
                slider_float(
                    "Scale", &settings.gui.rating_bad.scale, 0.5f, 2.0f, 0.1f
                )
            );
            settings.gui.rating_maybe.scale = settings.gui.rating_bad.scale;
            settings.gui.rating_good.scale = settings.gui.rating_bad.scale;
            mark_any(
                changed,
                ImGui::ColorEdit3("Bad", settings.gui.rating_bad.color.data())
            );
            mark_any(
                changed,
                ImGui::ColorEdit3(
                    "Maybe", settings.gui.rating_maybe.color.data()
                )
            );
            mark_any(
                changed,
                ImGui::ColorEdit3("Good", settings.gui.rating_good.color.data())
            );
            ImGui::TreePop();
        }
        ImGui::PopID();
        ImGui::PushID("Statistics");
        if (ImGui::TreeNode("Statistics")) {
            mark_fonts(
                changed,
                slider_float(
                    "Scale", &settings.gui.label.scale, 0.5f, 2.0f, 0.1f
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
    }
    if (ImGui::CollapsingHeader("Blood Money")) {
        mark_any(
            changed, ImGui::Checkbox("Use real time", &settings.hbm.real_time)
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
    }
    if (ImGui::CollapsingHeader("Absolution")) {
        mark_any(
            changed,
            ImGui::Checkbox(
                "Show SA rating details",
                &settings.hma.show_sa_details
            )
        );
        mark_any(
            changed,
            ImGui::Checkbox(
                "Show max Score rating only",
                &settings.hma.show_max_score_rating_only
            )
        );
        ImGui::SetItemTooltip(
            "Only show Shadow / No Shadow etc."
        );
        mark_any(
            changed,
            ImGui::Checkbox(
                "Show Score rating total", &settings.hma.show_score_total
            )
        );
        mark_any(
            changed,
            ImGui::Checkbox(
                "Show Score rating details", &settings.hma.show_score_details
            )
        );
        ImGui::PushID("Rating Mode");
        if (ImGui::TreeNode("Rating Mode")) {
            if (ImGui::Button("Original Game")) {
                settings.hma.rating_mode_unrated = settings::HMA::RatingMode::X;
                settings.hma.rating_mode_no_targets
                    = settings::HMA::RatingMode::SC;
                settings.hma.rating_mode_targets
                    = settings::HMA::RatingMode::SA_FALLBACK_SC;
                changed.any |= true;
            };
            ImGui::SameLine();
            if (ImGui::Button("Max Rating")) {
                settings.hma.rating_mode_unrated = settings::HMA::RatingMode::X;
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
    }
    if (ImGui::CollapsingHeader("Logging")) {
        bool log_changed = false;
        log_changed |= ImGui::Combo(
            "Log level", &settings.log.level, LOG_LEVEL_NAMES, 7
        );
        log_changed |= ImGui::Combo(
            "Flush level", &settings.log.flush_level, LOG_LEVEL_NAMES, 7
        );
        mark_any(changed, log_changed);
        if (log_changed)
            spdlog_set_level(settings.log.level, settings.log.flush_level);
    }
    return changed;
}