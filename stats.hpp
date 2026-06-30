#pragma once

#include <cstdint>
#include <optional>
#include <string>

enum class Status { GREEN, YELLOW, RED };

template <class T>
struct StatsValue {
    T value;
    std::optional<Status> status;
};

StatsValue<int32_t> stats_value(int32_t value, bool required = true);

enum class MapStage { pre, main, post };

// TODO split this up per game, use std::any for type erasure
struct _Stats_old {
    float time;
    int map;
    MapStage map_stage;
    int32_t difficulty;
    StatsValue<std::string> rating;
    StatsValue<int32_t> shots_fired;
    StatsValue<int32_t> headshots;
    StatsValue<int32_t> enemies_killed;
    StatsValue<int32_t> enemies_wounded;
    StatsValue<int32_t> innocents_killed;
    StatsValue<int32_t> innocents_wounded;
    StatsValue<int32_t> close_encounters;   // h2sa, hc
    StatsValue<int32_t> alerts;             // h2sa, hc
    StatsValue<int32_t> stealth;            // h2sa, hc
    StatsValue<int32_t> aggression;         // h2sa, hc
    StatsValue<int32_t> police_killed;      // bm
    StatsValue<int32_t> police_wounded;     // bm
    StatsValue<int32_t> frisk_failed;       // bm
    StatsValue<int32_t> cover_blown;        // bm
    StatsValue<int32_t> bodies_fnd;         // bm
    StatsValue<int32_t> target_bodies_fnd;  // bm
    StatsValue<int32_t> uncon_bodies_fnd;   // bm
    StatsValue<int32_t> witnesses;          // bm
    StatsValue<int32_t> on_camera;          // bm
    StatsValue<int32_t> cust_weapons_left;  // bm
    StatsValue<int32_t> suit_left;          // bm
    int32_t shots_hit;                      // bm
    int32_t accident_kills;                 // bm
};
