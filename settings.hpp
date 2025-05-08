#pragma once

#include <array>
#include <optional>
#include <string>

struct Font {
    std::string file;  // utf8 encoded .ttf file
    float size;        // font size in pixels
};

struct SettingsLog {
    int level{4};
    int flush_level{4};
};

struct SettingsGui {
    std::array<Font, 5> fonts;
    int title_font;   // font (0-4) to use for title
    int map_font;     // font (0-4) to use for map
    int time_font;    // font (0-4) to use for time
    int rating_font;  // font (0-4) to use for rating
    int table_font;   // font (0-4) to use for statistics table
};

struct Settings {
    SettingsLog log;
    SettingsGui gui;
};

std::optional<Settings> settings_load(int argc, char** argv);
