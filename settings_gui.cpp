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
        text_style_gui("Title", settings.gui.title, changed);
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
                "Always track Silent Assassin", &settings.hma.always_track_sa
            )
        );
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