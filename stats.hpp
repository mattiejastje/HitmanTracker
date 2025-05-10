#pragma once

#include <cinttypes>

enum class Status { GREEN, YELLOW, RED };

struct StatsValue {
    int32_t value;
    Status status;
};

struct Stats {
    float time;
    int map;
    Status silent_assassin;
    StatsValue difficulty;
    StatsValue shots_fired;
    StatsValue headshots;
    StatsValue enemies_killed;
    StatsValue enemies_wounded;
    StatsValue innocents_killed;
    StatsValue innocents_wounded;
    StatsValue close_encounters;   // h2sa
    StatsValue alerts;             // h2sa
    StatsValue police_killed;      // bm
    StatsValue police_wounded;     // bm
    StatsValue frisk_failed;       // bm
    StatsValue cover_blown;        // bm
    StatsValue bodies_fnd;         // bm
    StatsValue target_bodies_fnd;  // bm
    StatsValue uncon_bodies_fnd;   // bm
    StatsValue witnesses;          // bm
    StatsValue on_camera;          // bm
    StatsValue cust_weapons_left;  // bm
    StatsValue suit_left;          // bm
};
