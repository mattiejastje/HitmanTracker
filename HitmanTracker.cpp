#include <iostream>

#include "gui.hpp"
#include "settings.hpp"
#include "spdlog.hpp"

int main(int argc, char** argv) {
    std::cout << "HitmanTracker" << std::endl;
    std::cout << "-------------" << std::endl;
    std::cout << std::endl;
    spdlog_init();
    spdlog_set_level(4, 4);  // don't know levels yet, set to "info" for now
    auto settings = settings::load(argc, argv);
    if (settings) {
        spdlog_set_level(
            settings.value().log.level, settings.value().log.flush_level
        );
        gui_run(settings.value());
    }
    spdlog_shutdown();  // force flush before showing exit message
    std::cout << std::endl;
    std::cout << "Press enter to exit." << std::endl;
    std::cin.get();
    return 0;
}
