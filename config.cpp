#include "config.hpp"

#include <CLI/CLI.hpp>

#include "logger.hpp"

static const CLI::TransformPairs<spdlog::level::level_enum> log_level_map{
    {"trace", spdlog::level::trace},
    {"debug", spdlog::level::debug},
    {"info", spdlog::level::info},
    {"warn", spdlog::level::warn},
    {"warning", spdlog::level::warn},
    {"err", spdlog::level::err},
    {"error", spdlog::level::err},
    {"critical", spdlog::level::critical},
    {"off", spdlog::level::off},
};

static std::unique_ptr<CLI::App> make_app(
    spdlog::level::level_enum& log_level,
    spdlog::level::level_enum& log_flush_level
) {
    auto app = std::make_unique<CLI::App>("Hitman tracker", "HitmanTracker");
    app->add_option("--log-level", log_level, "Log level")
        ->capture_default_str()
        ->transform(CLI::CheckedTransformer(log_level_map, CLI::ignore_case));
    app->add_option(
           "--log-flush-level",
           log_flush_level,
           "On which level to flush the log"
    )
        ->capture_default_str()
        ->transform(CLI::CheckedTransformer(log_level_map, CLI::ignore_case));
    app->set_config(
        "--config", "HitmanTracker.ini", "Read options from ini file"
    );
    app->config_formatter(std::make_shared<CLI::ConfigINI>());
    return app;
}

void config_load(int argc, char** argv) {
    auto log_level = spdlog::level::info;
    auto log_flush_level = spdlog::level::off;
    auto app = make_app(log_level, log_flush_level);
    try {
        app->parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        LOG->warn("failed to parse options ({})", e.what());
    }
    LOG->set_level(log_level);
    LOG->flush_on(log_flush_level);
    g_config = std::make_unique<Config>();
}

void config_save() {
    auto log_level = g_logger ? g_logger->level() : spdlog::level::info;
    auto log_flush_level =
        g_logger ? g_logger->flush_level() : spdlog::level::off;
    auto app = make_app(log_level, log_flush_level);
    if (std::ofstream config_file{"HitmanTracker.ini"}) {
        config_file << app->config_to_str(true, true);
    } else {
        LOG->error("Unable to save config to HitmanTracker.ini");
    }
}
