#pragma once

#include <imgui.h>

#include <span>

#include "settings.hpp"

struct SettingsChanged {
    bool any{false};
    bool fonts{false};
    bool overlay_mode{false};
    bool reposition{false};
    bool logging{false};
};

SettingsChanged& operator|=(SettingsChanged& a, const SettingsChanged& b);

SettingsChanged settings_gui(settings::Settings& settings);
