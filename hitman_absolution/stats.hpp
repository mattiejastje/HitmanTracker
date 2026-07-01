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
    // store scores as strings since formatting is expensive
    StatsValue<std::string> score_rating;
    StatsValue<std::string> score_objective_complete;
    StatsValue<std::string> score_target_kill;
    StatsValue<std::string> score_spotted;
    StatsValue<std::string> score_evidence_removed;
    StatsValue<std::string> score_silent_assassin_bonus;
    StatsValue<std::string> score_signature_kill;
    StatsValue<std::string> score_silent_kill;
    StatsValue<std::string> score_headshot;
    StatsValue<std::string> score_body_hidden;
    StatsValue<std::string> score_civilian_casualty;
    StatsValue<std::string> score_non_target_casualty;
    StatsValue<std::string> score_pacification;
};

GameStatsSlow update_slow(const settings::HMA& hma, Version version);

GameStatsFast update_fast(Version version);

}  // namespace hitman_absolution
