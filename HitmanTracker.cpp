#include "config.hpp"
#include "logger.hpp"

using namespace std;

std::shared_ptr<spdlog::logger> g_logger = nullptr;
std::unique_ptr<Config> g_config = nullptr;

int main(int argc, char** argv) {
    logger_init();
    config_load(argc, argv);
    // TODO main application loop comes here
    config_save();
    return 0;
}
