#pragma once
#include "settings.hpp"

struct SettingsChanged {
    bool any{false};
    bool fonts{false};
    bool topmost{false};
};

// returns true if any setting changed this frame
SettingsChanged settings_gui(settings::Settings& settings);