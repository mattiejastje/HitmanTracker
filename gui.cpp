#include "gui.hpp"

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <shellapi.h>
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

static std::filesystem::path get_executable_directory() {
    wchar_t buffer[MAX_PATH];
    DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path();
}

static void imgui_open_file(std::wstring_view filename) {
    auto filepath = (get_executable_directory() / filename).c_str();
    ShellExecuteW(nullptr, L"open", filepath, nullptr, nullptr, SW_SHOWNORMAL);
}

constexpr auto WEBSITE_MAIN = "https://github.com/mattiejastje/HitmanTracker";

constexpr auto WEBSITE_ISSUES
    = "https://github.com/mattiejastje/HitmanTracker/issues";

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
              const bool show_border = !settings.gui.overlay_mode;
              if (show_border) {
                  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
                  ImGui::PushStyleColor(
                      ImGuiCol_Border, ImVec4{1.0f, 1.0f, 1.0f, 1.0f}
                  );
              }
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
              if (show_border) {
                  ImGui::PopStyleColor();
                  ImGui::PopStyleVar();
              }
              return imgui_app::AppWindowActions{};
          };

    spdlog::info("Running user interface");
    ImGui_ImplWin32_EnableDpiAwareness();
    std::shared_ptr<imgui_app::WindowClass> window_class
        = imgui_app::create_window_class(
            L"HitmanTrackerWindowClass", 0U, (HBRUSH)GetStockObject(BLACK_BRUSH)
        );
    constexpr UINT OVERLAY_EX_STYLE
        = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_NOACTIVATE;
    const POINT overlay_pos{settings.gui.overlay_x, settings.gui.overlay_y};
    auto stats = imgui_app::create_app_window(
        window_class,
        imgui_app::AppWindowSpec{
            .title = L"Hitman Tracker",
            .style = WS_POPUP,
            .ex_style = settings.gui.overlay_mode ? OVERLAY_EX_STYLE : 0U,
            .character_width = settings.gui.overlay_width,
            .character_height = settings.gui.overlay_height,
            .is_htclient_mapped_to_htcaption = true,  // drag
            .pos = overlay_pos,
        },
        settings.gui.font_size,
        hitman_common::make_font_specs(settings.gui),
        draw_stats
    );
    // persist position/size (character units) once the user finishes dragging
    stats->window->state->on_exit_size_move.push_back(
        [&settings, handle = stats->window->handle](const RECT& rect) {
            float dpiscale = ImGui_ImplWin32_GetDpiScaleForHwnd(handle);
            settings.gui.overlay_x = rect.left;
            settings.gui.overlay_y = rect.top;
            settings.gui.overlay_width = (rect.right - rect.left)
                                         / (settings.gui.font_size * dpiscale);
            settings.gui.overlay_height = (rect.bottom - rect.top)
                                          / (settings.gui.font_size * dpiscale);
            settings::save(settings);
        }
    );
    // keep the stored position roughly current if a DPI/monitor change moves it
    stats->window->state->on_dpi_changed.push_back(
        [&settings](float, const RECT& rect) {
            settings.gui.overlay_x = rect.left;
            settings.gui.overlay_y = rect.top;
            settings::save(settings);
        }
    );

    imgui_app::DrawFunc draw_main = [&settings, &game, &stats](
                                        imgui_app::AppWindow& aw, float dt
                                    ) {
        imgui_app::AppWindowActions actions{};
        if (ImGui::Begin(
                "Main",
                nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                    | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar
            )) {
            bool popup_reset = false;
            bool reset_confirmed = false;
            bool popup_about = false;
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("Settings")) {
                    if (ImGui::MenuItem("Reset")) {
                        popup_reset = true;
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Help")) {
                    if (ImGui::MenuItem("Report Bug")) {
                        ShellExecute(
                            nullptr,
                            "open",
                            WEBSITE_ISSUES,
                            nullptr,
                            nullptr,
                            SW_SHOWNORMAL
                        );
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Documentation"))
                        imgui_open_file(L"README.txt");
                    if (ImGui::MenuItem("Changelog"))
                        imgui_open_file(L"CHANGELOG.txt");
                    if (ImGui::MenuItem("License"))
                        imgui_open_file(L"LICENSE.txt");
                    ImGui::Separator();
                    if (ImGui::MenuItem("About...")) {
                        popup_about = true;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            if (popup_reset) ImGui::OpenPopup("Reset Settings");
            if (ImGui::BeginPopupModal(
                    "Reset Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize
                )) {
                ImGui::Text("Reset all settings to their default values?");
                if (ImGui::Button("Reset")) {
                    settings = settings::Settings{};
                    reset_confirmed = true;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
            if (popup_about) ImGui::OpenPopup("About");
            if (ImGui::BeginPopupModal(
                    "About", nullptr, ImGuiWindowFlags_AlwaysAutoResize
                )) {
                ImGui::TextUnformatted("Hitman Tracker");
                ImGui::SameLine();
                ImGui::TextDisabled("v%s", APP_VERSION);
                ImGui::Spacing();
                ImGui::TextUnformatted("Statistics tracker for Hitman games.");
                ImGui::Spacing();
                ImGui::TextUnformatted("Supported (Steam & GOG):");
                ImGui::BulletText("%s", "Hitman: Codename 47");
                ImGui::BulletText("%s", "Hitman 2: Silent Assassin");
                ImGui::BulletText("%s", "Hitman: Contracts");
                ImGui::BulletText("%s", "Hitman: Blood Money");
                ImGui::BulletText("%s", "Hitman: Absolution");
                ImGui::Spacing();
                ImGui::TextUnformatted("Website:");
                ImGui::BulletText("%s", WEBSITE_MAIN);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                        ShellExecute(
                            nullptr,
                            "open",
                            WEBSITE_MAIN,
                            nullptr,
                            nullptr,
                            SW_SHOWNORMAL
                        );
                    }
                }
                if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
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
            auto changed = settings_gui(settings);
            if (reset_confirmed) {
                changed.any |= true;
                changed.fonts |= true;
                changed.overlay_mode |= true;
            }
            if (changed.any) settings::save(settings);
            if (changed.fonts) {
                stats->ui->font_specs
                    = hitman_common::make_font_specs(settings.gui);
                actions.emplace_back(
                    stats.get(), imgui_app::AppWindowAction::UpdateUIScaling{}
                );
                float dpiscale
                    = ImGui_ImplWin32_GetDpiScaleForHwnd(stats->window->handle);
                UINT flags = SWP_NOZORDER | (reset_confirmed ? 0U : SWP_NOMOVE)
                             | SWP_NOACTIVATE;
                actions.emplace_back(
                    stats.get(),
                    imgui_app::AppWindowAction::SetWinPos{
                        .hwnd_insert_after = nullptr,
                        .x = 0,
                        .y = 0,
                        .cx = static_cast<int>(
                            settings.gui.overlay_width * settings.gui.font_size
                            * dpiscale
                        ),
                        .cy = static_cast<int>(
                            settings.gui.overlay_height * settings.gui.font_size
                            * dpiscale
                        ),
                        .flags = flags,
                    }
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