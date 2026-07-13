#include "gui.hpp"

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <spdlog/spdlog.h>
#include <tchar.h>

#include <CLI/CLI.hpp>
#include <algorithm>
#include <filesystem>
#include <map>
#include <tuple>

#include "color.hpp"
#include "game.hpp"
#include "hitman_common/gui.hpp"
#include "imgui_app/app_window.hpp"
#include "imgui_app/deviced3d.hpp"
#include "imgui_app/text.hpp"
#include "imgui_app/ui.hpp"
#include "imgui_app/window.hpp"
#include "mem/handle.hpp"
#include "settings_gui.hpp"
#include "signal.hpp"
#include "timer.hpp"

int gui_run(
    const std::vector<GameInfo>& registry, settings::Settings& settings
) {
    timer::PeriodicTimer timer_find_game{1.0};
    timer::PeriodicTimer timer_update_stats{0.1};
    std::optional<Game> game{};
    Signal frametime_signal{"fps", "frames per second"};
    Signal error_slow{"slow update failure rate", "%", 50.0f};
    Signal error_fast{"fast update failure rate", "%", 50.0f};
    Profiler profiler_slow{{"slow update time", "seconds"}};
    Profiler profiler_fast{{"fast update time", "seconds"}};

    imgui_app::TickFunc tick = [&registry,
                                &timer_find_game,
                                &timer_update_stats,
                                &game,
                                &frametime_signal,
                                &error_slow,
                                &error_fast,
                                &profiler_slow,
                                &profiler_fast](float dt) {
        frametime_signal.update(1 / dt, dt);
        if (timer_find_game.tick(dt)) {
            // try find game if none found yet
            if (!game || !is_process_running(game->handle.get())) {
                game = find_game(registry);
            };
            // try install hook if none installed yet
            if (game && !game->hook) {
                if (game->methods.hook_ready(
                        game->handle.get(), game->base_ptrs
                    )) {
                    game->hook
                        = game->methods.hook(game->handle, game->base_ptrs);
                    if (game->hook) {
                        spdlog::info("Game is now tracked");
                    } else {
                        spdlog::error("Game hook failed");
                    }
                } else {
                    spdlog::debug("Game not yet ready for tracking...");
                }
            }
        }
        if (timer_update_stats.tick(dt)) {
            if (game && game->hook) {
                auto scoped_slow = ScopedProfiler{profiler_slow, dt};
                auto ok = game->methods.update_slow(
                    game->exe_path,
                    game->handle.get(),
                    game->base_ptrs,
                    game->hook->label_ptrs,
                    game->remote_state,
                    game->stats
                );
                error_slow.update(100.0f * static_cast<float>(!ok), dt);
            }
        }
        if (game && game->hook) {
            auto scoped_fast = ScopedProfiler{profiler_fast, dt};
            bool ok = game->methods.update_fast(
                game->handle.get(),
                game->base_ptrs,
                game->hook->label_ptrs,
                game->stats
            );
            error_fast.update(100.0f * static_cast<float>(!ok), dt);
        }
    };

    imgui_app::DrawFunc draw_stats
        = [&settings, &game](imgui_app::AppWindow& aw, float dt) {
              if (ImGui::Begin(
                      "Stats",
                      nullptr,
                      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                          | ImGuiWindowFlags_NoMove
                  )) {
                  if (game && game->hook) {
                      game->methods.gui(aw.ui->fonts, game->stats);
                  }
                  ImGui::End();
              }
              return imgui_app::AppWindowActions{};
          };

    spdlog::info("Running user interface");
    ImGui_ImplWin32_EnableDpiAwareness();
    std::shared_ptr<imgui_app::WindowClass> window_class
        = imgui_app::create_window_class(0U);
    constexpr UINT OVERLAY_EX_STYLE
        = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_NOACTIVATE;
    auto stats = imgui_app::create_app_window(
        window_class,
        imgui_app::AppWindowSpec{
            .title = L"Hitman Tracker",
            .style = WS_POPUP,
            .ex_style = settings.gui.overlay_mode ? OVERLAY_EX_STYLE : 0U,
            .character_width = 15,
            .character_height = 30,
            .is_htclient_mapped_to_htcaption = true,  // drag
            .pos = std::nullopt,
        },
        settings.gui.font_size,
        hitman_common::make_font_specs(settings.gui),
        draw_stats
    );

    imgui_app::DrawFunc draw_main = [&settings, &game, &stats](
                                        imgui_app::AppWindow& aw, float dt
                                    ) {
        imgui_app::AppWindowActions actions{};
        if (ImGui::Begin(
                "Main",
                nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                    | ImGuiWindowFlags_NoMove
            )) {
            if (game && !game->hook) {
                ImGui::Text(
                    "Connecting to %s...",
                    game->exe_path.filename().string().c_str()
                );
            } else if (game && game->hook) {
                ImGui::Text(
                    "Connected to %s",
                    game->exe_path.filename().string().c_str()
                );
            } else {
                ImGui::Text("No game detected");
            }
            ImGui::SeparatorText("Settings");
            auto changed = settings_gui(settings);
            if (changed.any) settings::save(settings);
            if (changed.fonts) {
                stats->ui->font_specs
                    = hitman_common::make_font_specs(settings.gui);
                actions.emplace_back(
                    stats.get(), imgui_app::AppWindowAction::UpdateUIScaling{}
                );
            }
            if (changed.overlay_mode) {
                spdlog::debug("Overlay mode is {}", settings.gui.overlay_mode);
                auto ex_style
                    = settings.gui.overlay_mode ? OVERLAY_EX_STYLE : 0U;
                UINT flags = SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED
                             | SWP_NOACTIVATE;
                actions.emplace_back(
                    stats.get(),
                    imgui_app::AppWindowAction::SetWinLongPtr{
                        .index = GWL_EXSTYLE,
                        .value = ex_style,
                    }
                );
                if (ex_style & WS_EX_LAYERED) {
                    actions.emplace_back(
                        stats.get(),
                        imgui_app::AppWindowAction::SetTransparentColorKey{}
                    );
                }
                actions.emplace_back(
                    stats.get(),
                    imgui_app::AppWindowAction::SetWinPos{
                        .hwnd_insert_after = settings.gui.overlay_mode
                                                 ? HWND_TOPMOST
                                                 : HWND_NOTOPMOST,
                        .flags = flags,
                    }
                );
            }
            ImGui::End();
        }
        return actions;
    };

    auto main = imgui_app::create_app_window(
        window_class,
        imgui_app::AppWindowSpec{
            .title = L"Hitman Tracker - Settings",
            .style = WS_OVERLAPPEDWINDOW,
            .ex_style = 0U,
            .character_width = 48,
            .character_height = 27,
            .is_htclient_mapped_to_htcaption = false,
            .pos = std::nullopt,
        },
        12.0,
        std::vector<imgui_app::FontSpec>{
            {"fonts/proggyforever/ProggyForever-Regular.ttf", 12.0f}
        },
        draw_main
    );
    std::vector<imgui_app::AppWindow*> aws{stats.get(), main.get()};
    imgui_app::run(tick, aws);
    spdlog::debug("Cleanup...");
    return 0;
}