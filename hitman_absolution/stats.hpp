#pragma once

#include <filesystem>

#include "../game.hpp"
#include "../settings.hpp"
#include "version.hpp"

namespace hitman_absolution {

enum class CheckpointType { UNRATED, NO_TARGETS, TARGETS };

struct Stats {
    int64_t start_time;  // start of mission in game ticks
    float time;
    int map;
    MapStage map_stage;
    int32_t difficulty;
    CheckpointType checkpoint_type;
    StatsValue<std::string> rating;
    StatsValue<std::string> score_rating;
    StatsValue<int32_t> score_objective_complete;
    StatsValue<int32_t> score_target_kill;
    StatsValue<int32_t> score_spotted;
    StatsValue<int32_t> score_evidence_removed;
    StatsValue<int32_t> score_silent_assassin_bonus;
    StatsValue<int32_t> score_signature_kill;
    StatsValue<int32_t> score_silent_kill;
    StatsValue<int32_t> score_headshot;
    StatsValue<int32_t> score_body_hidden;
    StatsValue<int32_t> score_civilian_casualty;
    StatsValue<int32_t> score_non_target_casualty;
    StatsValue<int32_t> score_pacification;
};

GameStatsSlow update_slow(const settings::HMA& hma, Version version);

GameStatsFast update_fast(Version version);

}  // namespace hitman_absolution
