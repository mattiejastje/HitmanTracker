#include "settings.hpp"

#include <CLI/CLI.hpp>
#include <format>

#include "logging.hpp"

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
    app->add_option("--font-size", settings.gui.font_size, "Font size")
        ->capture_default_str();
    app->add_option("--bg-color", settings.gui.bg_color, "Background color")
        ->capture_default_str();
    add_text_style_options(*app, "title", "title", settings.gui.title);
    add_text_style_options(*app, "map", "map", settings.gui.map);
    add_text_style_options(*app, "time", "time", settings.gui.time);
    add_text_style_options(
        *app, "rating-good", "good rating", settings.gui.rating_good
    );
    add_text_style_options(
        *app, "rating-maybe", "maybe rating", settings.gui.rating_maybe
    );
    add_text_style_options(
        *app, "rating-bad", "bad rating", settings.gui.rating_bad
    );
    add_text_style_options(*app, "label", "labels", settings.gui.label);
    add_text_style_options(*app, "value", "values", settings.gui.value);
    return app;
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
        logging::error("Settings error ({}: {})", e.get_name(), e.what());
        return {};
    }
    std::cout << app->config_to_str(false, false);
    return settings;
}
