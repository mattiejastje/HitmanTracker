#include "logger.hpp"
#include "settings.hpp"

using namespace std;

std::unique_ptr<Settings> g_settings = nullptr;

int main(int argc, char** argv) {
    logger_init();
    settings_load(argc, argv);
    // TODO main application loop comes here
    settings_save();
    return 0;
}
