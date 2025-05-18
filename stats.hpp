#pragma once

#include <cstdint>

enum class Status { GREEN, YELLOW, RED };

struct StatsValue {
    int32_t value;
    Status status;
};

StatsValue stats_value(int32_t value, bool required = true);

enum class MapStage { pre, main, post };

struct Stats {
    float time;
    int map;
    MapStage map_stage;
    Status silent_assassin;
    int32_t difficulty;
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
    StatsValue spotted;            // hma
    StatsValue evidence_left;      // hma
};
