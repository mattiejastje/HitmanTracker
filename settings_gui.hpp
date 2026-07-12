#pragma once

#include <imgui.h>

#include <span>

#include "settings.hpp"

struct SettingsChanged {
    bool any{false};
    bool fonts{false};
    bool overlay_mode{false};
};

SettingsChanged settings_gui(settings::Settings& settings);
