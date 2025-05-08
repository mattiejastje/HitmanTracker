#pragma once

#include <array>
#include <optional>
#include <string>

struct Font {
    std::string path;  // utf8 encoded .ttf path
    float size;        // font size in pixels
};

struct Settings {
    int log_level{4};
    int log_flush_level{4};
    std::array<Font, 5> fonts;
    int font_title;   // font (0-4) to use for title
    int font_map;     // font (0-4) to use for map
    int font_time;    // font (0-4) to use for time
    int font_rating;  // font (0-4) to use for rating
    int font_table;   // font (0-4) to use for statistics table
};

std::optional<Settings> settings_load(int argc, char** argv);
bool settings_save(Settings& settings);
