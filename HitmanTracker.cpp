#include "gui.hpp"
#include "settings.hpp"
#include "spdlog.hpp"

int main(int argc, char** argv) {
    spdlog_init();
    spdlog_set_level(4, 4);  // don't know levels yet, set to "info" for now
    auto settings = settings_load(argc, argv).value_or(Settings{});
    spdlog_set_level(settings.log_level, settings.log_flush_level);
    gui_run(settings);
    settings_save(settings);
    return 0;
}
