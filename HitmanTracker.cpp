#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

#include <CLI/CLI.hpp>

using namespace std;

const CLI::TransformPairs<spdlog::level::level_enum> log_level_map{
    {"trace", spdlog::level::trace},
    {"debug", spdlog::level::debug},
    {"info", spdlog::level::info},
    {"warn", spdlog::level::warn},
    {"err", spdlog::level::err},
    {"error", spdlog::level::err},
    {"critical", spdlog::level::critical},
    {"off", spdlog::level::off},
};

int main(int argc, char** argv) {
    CLI::App app{"Hitman tracker"};
    auto log_level = spdlog::level::info;
    auto log_flush_on = spdlog::level::off;
    app.add_option("--log-level", log_level, "Log level")
        ->capture_default_str()
        ->transform(CLI::CheckedTransformer(log_level_map, CLI::ignore_case));
    app.add_option(
           "--log-flush-on", log_flush_on, "On which level to flush the log"
    )
        ->capture_default_str()
        ->transform(CLI::CheckedTransformer(log_level_map, CLI::ignore_case));
    app.set_config(
        "--config", "HitmanTracker.ini", "Read options from ini file"
    );
    app.config_formatter(std::make_shared<CLI::ConfigINI>());
    CLI11_PARSE(app, argc, argv);
    auto logger = spdlog::rotating_logger_mt(
        "HitmanTracker", "HitmanTracker.log", 10000000, 5
    );
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%t] [%^%l%$] %v");
    logger->set_level(log_level);
    logger->flush_on(log_flush_on);
    if (std::ofstream config_file{"HitmanTracker.ini"}) {
        config_file << app.config_to_str(true, true);
    } else {
        logger->error("Unable to save config to HitmanTracker.ini");
    }
    return 0;
}
