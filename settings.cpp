#include "settings.hpp"

#include <spdlog/spdlog.h>

#include <CLI/CLI.hpp>
#include <format>

using namespace settings;

static const CLI::TransformPairs<int> log_level_map{
    {"trace", 6},
    {"debug", 5},
    {"info", 4},
    {"warn", 3},
    {"error", 2},
    {"critical", 1},
    {"off", 0},
};

static void add_text_style_options(
    CLI::App& app, std::string name, std::string desc, TextStyle& style
) {
    app.add_option(
           std::format("--{}-font", name),
           style.file,
           std::format("Font for {}", desc)
    )
        ->check(CLI::ExistingFile)
        ->default_str(style.file.string());
    app.add_option(
           std::format("--{}-scale", name),
           style.scale,
           std::format("Scale for {}", desc)
    )
        ->check(CLI::PositiveNumber)
        ->capture_default_str();
    app.add_option(
           std::format("--{}-color", name),
           style.color,
           std::format("Color for {}", desc)
    )
        ->capture_default_str();
};

static std::unique_ptr<CLI::App> make_app(Settings& settings) {
    auto app = std::make_unique<CLI::App>("Hitman tracker", "HitmanTracker");
    app->set_config(
        "--config", "HitmanTracker.ini", "Read options from ini file"
    );
    auto log_group
        = app->add_option_group("Logging", "Options related to logging");
    log_group->add_option("--log-level", settings.log.level, "Log level")
        ->capture_default_str()
        ->transform(CLI::CheckedTransformer(log_level_map, CLI::ignore_case));
    log_group
        ->add_option(
            "--log-flush-level",
            settings.log.flush_level,
            "On which level to flush the log"
        )
        ->capture_default_str()
        ->transform(CLI::CheckedTransformer(log_level_map, CLI::ignore_case));
    auto gui_group = app->add_option_group(
        "GUI", "Options related to the graphical user interface"
    );
    gui_group
        ->add_option("--topmost", settings.gui.topmost, "Force topmost window")
        ->capture_default_str();
    gui_group->add_option("--font-size", settings.gui.font_size, "Font size")
        ->capture_default_str();
    gui_group
        ->add_option("--bg-color", settings.gui.bg_color, "Background color")
        ->capture_default_str();
    gui_group
        ->add_option(
            "--show-game-version",
            settings.gui.show_game_version,
            "Show game version (i.e. steam, gog, ...)"
        )
        ->capture_default_str();
    add_text_style_options(*gui_group, "title", "title", settings.gui.title);
    add_text_style_options(*gui_group, "version", "version", settings.gui.version);
    add_text_style_options(*gui_group, "map", "map", settings.gui.map);
    add_text_style_options(*gui_group, "time", "time", settings.gui.time);
    add_text_style_options(
        *gui_group, "rating-good", "good rating", settings.gui.rating_good
    );
    add_text_style_options(
        *gui_group, "rating-maybe", "maybe rating", settings.gui.rating_maybe
    );
    add_text_style_options(
        *gui_group, "rating-bad", "bad rating", settings.gui.rating_bad
    );
    add_text_style_options(*gui_group, "label", "labels", settings.gui.label);
    add_text_style_options(*gui_group, "value", "values", settings.gui.value);
    auto hbm_group = app->add_option_group(
        "Hitman Blood Money", "Options related to Hitman Blood Money"
    );
    hbm_group
        ->add_option(
            "--hbm-real-time",
            settings.hbm.real_time,
            "Use real time instead of mission time (mission time is 2.4% fast)"
        )
        ->capture_default_str();
    hbm_group
        ->add_option(
            "--hbm-show-shots-hit",
            settings.hbm.show_shots_hit,
            "Show shots hit"
        )
        ->capture_default_str();
    hbm_group
        ->add_option(
            "--hbm-show-accident-kills",
            settings.hbm.show_accident_kills,
            "Show accident kills"
        )
        ->capture_default_str();
    auto hma_group = app->add_option_group(
        "Hitman Absolution", "Options related to Hitman Absolution"
    );
    hma_group
        ->add_option(
            "--hma-show-sa-details",
            settings.hma.show_sa_details,
            "Show Silent Assassin rating details"
        )
        ->capture_default_str();
    hma_group
        ->add_option(
            "--hma-show-max-rating-only",
            settings.hma.show_max_score_rating_only,
            "Show only if maximum Score rating was achieved or not"
        )
        ->capture_default_str();
    hma_group
        ->add_option(
            "--hma-show-score-total",
            settings.hma.show_score_total,
            "Show Score rating total"
        )
        ->capture_default_str();
    hma_group
        ->add_option(
            "--hma-show-score-details",
            settings.hma.show_score_details,
            "Show Score rating details"
        )
        ->capture_default_str();
    hma_group
        ->add_option(
            "--hma-apply-bonus",
            settings.hma.apply_bonus,
            "Apply score bonus based on difficulty and challenges completed"
        )
        ->capture_default_str();
    hma_group
        ->add_option(
            "--hma-rating-mode-unrated",
            settings.hma.rating_mode_unrated,
            "Rating mode for unrated checkpoints"
        )
        ->capture_default_str();
    hma_group
        ->add_option(
            "--hma-rating-mode-no-targets",
            settings.hma.rating_mode_no_targets,
            "Rating mode for rated checkpoints without targets"
        )
        ->capture_default_str();
    hma_group
        ->add_option(
            "--hma-rating-mode-targets",
            settings.hma.rating_mode_targets,
            "Rating mode for rated checkpoints with targets"
        )
        ->capture_default_str();
    return app;
}

static void _save(const CLI::App& app) {
    std::string content{};
    try {
        content = app.config_to_str(true, true);
    } catch (const CLI::Error& e) {
        spdlog::error(
            "Failed to save configuration ({}: {})", e.get_name(), e.what()
        );
        return;
    }
    try {
        std::ofstream out("HitmanTracker.ini");
        out.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        out << content;
        out.close();
        spdlog::debug("Configuration saved");
    } catch (const std::exception& e) {
        spdlog::warn("Failed to save configuration: {}", e.what());
    }
}

std::optional<Settings> settings::load(int argc, char** argv) {
    Settings settings{};
    auto app = make_app(settings);
    argv = app->ensure_utf8(argv);
    try {
        app->parse(argc, argv);
    } catch (const CLI::CallForHelp& e) {
        std::cout << app->help();
        return {};
    } catch (const CLI::ParseError& e) {
        spdlog::error("Settings error ({}: {})", e.get_name(), e.what());
        spdlog::warn("Fix HitmanTracker.ini or delete it to recreate.");
        spdlog::warn("Falling back on default settings for now.");
        return Settings{};
    }
    std::filesystem::path ini_file{"HitmanTracker.ini"};
    if (!std::filesystem::exists(ini_file)) {
        spdlog::info("Creating HitmanTracker.ini");
        _save(*app);
    }
    return settings;
}

void settings::save(Settings& settings) {
    auto app = make_app(settings);
    std::filesystem::path ini_file{"HitmanTracker.ini"};
    _save(*app);
};