#include "gui.hpp"
#include "settings.hpp"
#include "spdlog.hpp"

int main(int argc, char** argv) {
    spdlog_init();
    spdlog_set_level(4, 4);  // don't know levels yet, set to "info" for now
    auto settings = settings::load(argc, argv);
    if (settings) {
        spdlog_set_level(
            settings.value().log.level, settings.value().log.flush_level
        );
        gui_run(settings.value());
    }
    return 0;
}
