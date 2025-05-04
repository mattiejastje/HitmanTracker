#pragma once

constexpr auto MAX_STATS_VALUES = 20;

enum SilentAssassin { YES, MAYBE, NO };

struct Stats {
    float time;
    int map;
    SilentAssassin silent_assassin;
    int values[MAX_STATS_VALUES];
};
