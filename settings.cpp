#include "settings.hpp"

#include <CLI/CLI.hpp>
#include <format>

#include "logging.hpp"

static const CLI::TransformPairs<int> log_level_map{
    {"trace", 6},
    {"debug", 5},
    {"info", 4},
    {"warn", 3},
    {"warning", 3},
    {"err", 2},
    {"error", 2},
    {"crit", 1},
    {"critical", 1},
    {"off", 0},
};

static std::unique_ptr<CLI::App> make_app(Settings& settings) {
    auto app = std::make_unique<CLI::App>("Hitman tracker", "HitmanTracker");
    app->add_option("--log-level", settings.log.level, "Log level")
        ->capture_default_str()
        ->transform(CLI::CheckedTransformer(log_level_map, CLI::ignore_case));
    app->add_option(
           "--log-flush-level",
           settings.log.flush_level,
           "On which level to flush the log"
    )
        ->capture_default_str()
        ->transform(CLI::CheckedTransformer(log_level_map, CLI::ignore_case));
    app->set_config(
        "--config", "HitmanTracker.ini", "Read options from ini file"
    );
    for (int i = 0; i < 5; i++) {
        app->add_option(
               std::format("--font-{}-path", i),
               settings.gui.fonts[i].file,
               std::format("The .ttf file for font {} (empty for built-in)", i)
        )
            ->capture_default_str();
        app->add_option(
               std::format("--font-{}-size", i),
               settings.gui.fonts[i].size,
               std::format("Size of font {} if resizeable", i)
        )
            ->capture_default_str();
    }
    app->add_option("--font-title", settings.gui.title_font, "Title font")
        ->capture_default_str();
    app->add_option("--font-map", settings.gui.map_font, "Map font")
        ->capture_default_str();
    app->add_option("--font-time", settings.gui.time_font, "Time font")
        ->capture_default_str();
    app->add_option("--font-rating", settings.gui.rating_font, "Rating font")
        ->capture_default_str();
    app->add_option("--font-table", settings.gui.table_font, "Table font")
        ->capture_default_str();
    app->config_formatter(std::make_shared<CLI::ConfigINI>());
    return app;
}

std::optional<Settings> settings_load(int argc, char** argv) {
    Settings settings{};
    auto app = make_app(settings);
    argv = app->ensure_utf8(argv);
    try {
        app->parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        logging::warn("Failed to parse settings ({})", e.what());
        return {};
    }
    return settings;
}

bool settings_save(Settings& settings) {
    auto app = make_app(settings);
    if (std::ofstream config_file{"HitmanTracker.ini"}) {
        config_file << app->config_to_str(true, true);
        logging::debug("Settings saved to HitmanTracker.ini");
        return true;
    } else {
        logging::error("Unable to save settings to HitmanTracker.ini");
        return false;
    }
}
