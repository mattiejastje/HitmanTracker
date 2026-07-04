#pragma once

#include <imgui.h>

#include <span>

#include "settings.hpp"

struct SettingsChanged {
    bool any{false};
    bool fonts{false};
    bool topmost{false};
};

SettingsChanged settings_gui(settings::Settings& settings);
