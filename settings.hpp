#pragma once

#include <memory>

struct Settings {
    // TODO add configuration variables here
};

Settings settings_load(int argc, char** argv);
void settings_save(const Settings& settings);
