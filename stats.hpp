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

// only for HMA
enum class CheckpointType { UNRATED, NO_TARGETS, TARGETS };

enum class MapStage { pre, main, post };

// TODO split this up per game, use std::any for type erasure
struct Stats {
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
    CheckpointType checkpoint_type;         // hma: checkpoint type needed for display
    int32_t score_for_max_rating;           // hma: score needed for max rating
    int32_t score_total;                    // hma: actual score
    std::string score_rating;               // hma: actual rating
    StatsValue<int32_t> score_objective_complete;     // hma
    StatsValue<int32_t> score_target_kill;            // hma
    StatsValue<int32_t> score_spotted;                // hma
    StatsValue<int32_t> score_evidence_removed;       // hma
    StatsValue<int32_t> score_silent_assassin_bonus;  // hma
    StatsValue<int32_t> score_signature_kill;         // hma
    StatsValue<int32_t> score_silent_kill;            // hma
    StatsValue<int32_t> score_headshot;               // hma
    StatsValue<int32_t> score_body_hidden;            // hma
    StatsValue<int32_t> score_civilian_casualty;      // hma
    StatsValue<int32_t> score_non_target_casualty;    // hma
    StatsValue<int32_t> score_pacification;           // hma
};
