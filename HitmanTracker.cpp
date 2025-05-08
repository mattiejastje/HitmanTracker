#include "logger.hpp"
#include "settings.hpp"
#include "gui.hpp"

int main(int argc, char** argv) {
    logger_init();
    auto settings = settings_load(argc, argv);
    gui_run(settings);
    settings_save(settings);
    return 0;
}
