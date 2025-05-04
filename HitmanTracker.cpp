#include "logger.hpp"
#include "settings.hpp"
#include "gui.hpp"
#include "game.hpp"

using namespace std;

std::unique_ptr<Settings> g_settings = nullptr;

int main(int argc, char** argv) {
    logger_init();
    settings_load(argc, argv);
    gui_run();
    settings_save();
    return 0;
}
