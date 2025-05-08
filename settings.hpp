#pragma once

#include <optional>

struct Settings {
    int log_level;
    int log_flush_level;
};

std::optional<Settings> settings_load(int argc, char** argv);
bool settings_save(Settings settings);
