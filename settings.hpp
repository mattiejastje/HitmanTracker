#pragma once

#include <optional>
#include <string>

struct Font {
    std::string path;  // utf8 encoded .ttf path
    float size;        // font size in pixels
};

struct Settings {
    int log_level{4};
    int log_flush_level{4};
    Font font{"", 13};
};

std::optional<Settings> settings_load(int argc, char** argv);
bool settings_save(Settings& settings);
