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

static void shell_open_url(const char* url) {
    ShellExecuteA(nullptr, "open", url, nullptr, nullptr, SW_SHOWNORMAL);
}

static void shell_open_file(const wchar_t* filename) {
    static wchar_t buffer[MAX_PATH];
    DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    auto path = std::filesystem::path(buffer).parent_path().wstring();
    ShellExecuteW(
        nullptr, L"open", filename, nullptr, path.c_str(), SW_SHOWNORMAL
    );
}

constexpr UINT OVERLAY_EX_STYLE
    = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_NOACTIVATE;

constexpr auto WEBSITE_MAIN = "https://github.com/mattiejastje/HitmanTracker";

constexpr auto WEBSITE_ISSUES
    = "https://github.com/mattiejastje/HitmanTracker/issues";

template <class F>
void modal_popup(const char* name, bool open, F&& body) {
    if (open) ImGui::OpenPopup(name);
    if (ImGui::BeginPopupModal(
            name, nullptr, ImGuiWindowFlags_AlwaysAutoResize
        )) {
        body();
        ImGui::EndPopup();
    }
}

struct MenuActions {
    bool open_reset{false};
    bool open_about{false};
};

static MenuActions draw_main_menu() {
    MenuActions menu_actions;
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Settings")) {
            if (ImGui::MenuItem("Reset")) {
                menu_actions.open_reset = true;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Report Bug")) shell_open_url(WEBSITE_ISSUES);
            ImGui::Separator();
            if (ImGui::MenuItem("Documentation"))
                shell_open_file(L"README.txt");
            if (ImGui::MenuItem("Changelog")) shell_open_file(L"CHANGELOG.txt");
            if (ImGui::MenuItem("License")) shell_open_file(L"LICENSE.txt");
            ImGui::Separator();
            if (ImGui::MenuItem("About...")) {
                menu_actions.open_about = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    return menu_actions;
}

static bool draw_popup_reset(bool open) {
    bool reset_pressed = false;
    modal_popup("Reset Settings", open, [&reset_pressed]() {
        ImGui::Text("Reset all settings to their default values?");
        if (ImGui::Button("Reset")) {
            reset_pressed = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
    });
    return reset_pressed;
}

static void draw_popup_about(bool open) {
    modal_popup("About", open, []() {
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
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                shell_open_url(WEBSITE_MAIN);
        }
        if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
    });
};

static void draw_main_status(const char* game_exe, bool hooked) {
    if (game_exe) {
        ImGui::Text(
            hooked ? "Connected to %s" : "Connecting to %s...", game_exe
        );
    } else {
        ImGui::Text("No game detected");
    }
}

static imgui_app::AppWindowActions draw_main_side_effects(
    imgui_app::AppWindow* stats,
    const SettingsChanged& changed,
    settings::Settings& settings
) {
    imgui_app::AppWindowActions actions{};
    if (changed.any) settings::save(settings);
    if (changed.fonts) {
        stats->ui->font_specs = hitman_common::make_font_specs(settings.gui);
        actions.emplace_back(
            stats, imgui_app::AppWindowAction::UpdateUIScaling{}
        );
        float dpiscale
            = ImGui_ImplWin32_GetDpiScaleForHwnd(stats->window->handle);
        actions.emplace_back(
            stats,
            imgui_app::AppWindowAction::SetWinPos{
                .cx = static_cast<int>(
                    settings.gui.overlay_width * settings.gui.font_size
                    * dpiscale
                ),
                .cy = static_cast<int>(
                    settings.gui.overlay_height * settings.gui.font_size
                    * dpiscale
                ),
                .flags = SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE,
            }
        );
    }
    if (changed.overlay_mode) {
        spdlog::debug("Overlay mode is {}", settings.gui.overlay_mode);
        auto ex_style = settings.gui.overlay_mode ? OVERLAY_EX_STYLE : 0U;
        actions.emplace_back(
            stats,
            imgui_app::AppWindowAction::SetWinLongPtr{
                .index = GWL_EXSTYLE,
                .value = ex_style,
            }
        );
        if (ex_style & WS_EX_LAYERED) {
            actions.emplace_back(
                stats,
                imgui_app::AppWindowAction::SetLayeredWinAttrs{
                    .color = RGB(0, 0, 0),
                    .flags = LWA_COLORKEY,
                }
            );
        }
        actions.emplace_back(
            stats,
            imgui_app::AppWindowAction::SetWinPos{
                .hwnd_insert_after
                = settings.gui.overlay_mode ? HWND_TOPMOST : HWND_NOTOPMOST,
                .flags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE,
            }
        );
    }
    if (changed.reposition) {
        actions.emplace_back(
            stats,
            imgui_app::AppWindowAction::SetWinPos{
                .x = settings.gui.overlay_x,
                .y = settings.gui.overlay_y,
                .flags = SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE,
            }
        );
    }
    return actions;
}

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
    const POINT overlay_pos{settings.gui.overlay_x, settings.gui.overlay_y};
    auto stats = imgui_app::create_app_window(
        window_class,
        imgui_app::AppWindowSpec{
            .title = L"Hitman Tracker",
            .style = WS_POPUP,
            .ex_style = settings.gui.overlay_mode ? OVERLAY_EX_STYLE : 0U,
            .character_width = settings.gui.overlay_width,
            .character_height = settings.gui.overlay_height,
            .pos = overlay_pos,
        },
        settings.gui.font_size,
        hitman_common::make_font_specs(settings.gui),
        draw_stats
    );

    imgui_app::OnMessage on_message_stats
        = [&settings, handle = stats->window->handle](
              UINT msg, WPARAM wparam, LPARAM lparam
          ) -> std::optional<LRESULT> {
        switch (msg) {
            case WM_EXITSIZEMOVE: {
                // save position/size (character units)
                // once the user finishes dragging
                float dpiscale = ImGui_ImplWin32_GetDpiScaleForHwnd(handle);
                RECT rect;
                if (::GetWindowRect(handle, &rect)) {
                    settings.gui.overlay_x = rect.left;
                    settings.gui.overlay_y = rect.top;
                    settings.gui.overlay_width
                        = (rect.right - rect.left)
                          / (settings.gui.font_size * dpiscale);
                    settings.gui.overlay_height
                        = (rect.bottom - rect.top)
                          / (settings.gui.font_size * dpiscale);
                    settings::save(settings);
                }
                break;
            }
            case WM_DPICHANGED: {
                // keep the stored position roughly current
                // if a DPI/monitor change moves it
                const RECT& rect = *reinterpret_cast<RECT*>(lparam);
                settings.gui.overlay_x = rect.left;
                settings.gui.overlay_y = rect.top;
                settings::save(settings);
                break;
            }
            case WM_SETCURSOR:
                // show move icon when hovering over client area
                if (LOWORD(lparam) == HTCAPTION) {
                    SetCursor(LoadCursor(NULL, IDC_SIZEALL));
                    return TRUE;
                }
                break;
            case WM_NCHITTEST: {
                // treat client area (HTCLIENT) as title area (HTCAPTION)
                // make border draggable (HTTOP/BOTTOM/LEFT/RIGHT...)
                float dpiscale = ImGui_ImplWin32_GetDpiScaleForHwnd(handle);
                const int border = std::lround(5 * dpiscale);
                POINT pt;
                GetCursorPos(&pt);
                ScreenToClient(handle, &pt);
                RECT rc;
                GetClientRect(handle, &rc);
                if (pt.x < border && pt.y < border) return HTTOPLEFT;
                if (pt.x > rc.right - border && pt.y < border)
                    return HTTOPRIGHT;
                if (pt.x < border && pt.y > rc.bottom - border)
                    return HTBOTTOMLEFT;
                if (pt.x > rc.right - border && pt.y > rc.bottom - border)
                    return HTBOTTOMRIGHT;
                if (pt.x < border) return HTLEFT;
                if (pt.x > rc.right - border) return HTRIGHT;
                if (pt.y < border) return HTTOP;
                if (pt.y > rc.bottom - border) return HTBOTTOM;
                auto hit = ::DefWindowProcW(handle, msg, wparam, lparam);
                return hit == HTCLIENT ? HTCAPTION : hit;
            }
        }
        return std::nullopt;
    };

    stats->window->state->on_message.push_back(on_message_stats);

    imgui_app::DrawFunc draw_main = [&settings, &game, &stats](
                                        imgui_app::AppWindow& aw, float dt
                                    ) -> imgui_app::AppWindowActions {
        if (ImGui::Begin(
                "Main",
                nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                    | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar
            )) {
            auto menu_actions = draw_main_menu();
            SettingsChanged changed{};
            if (draw_popup_reset(menu_actions.open_reset)) {
                settings = settings::Settings{};
                changed |= SettingsChanged{
                    .any = true,
                    .fonts = true,
                    .overlay_mode = true,
                    .reposition = true
                };
            }
            draw_popup_about(menu_actions.open_about);
            draw_main_status(
                game ? game->exe_path.filename().string().c_str() : nullptr,
                game ? static_cast<bool>(game->hook) : false
            );
            changed |= settings_gui(settings);
            ImGui::End();
            return draw_main_side_effects(stats.get(), changed, settings);
        }
        return {};
    };

    auto main = imgui_app::create_app_window(
        window_class,
        imgui_app::AppWindowSpec{
            .title = L"Hitman Tracker - Settings",
            .style = WS_OVERLAPPEDWINDOW,
            .ex_style = 0U,
            .character_width = 48,
            .character_height = 27,
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