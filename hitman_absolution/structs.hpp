#pragma once

#include <array>
#include <cstdint>
#include <mempeep/descriptors.hpp>
#include <optional>
#include <string>
#include <vector>

using namespace mempeep;

namespace hitman_absolution::structs {

constexpr int32_t NUM_DIFFICULTIES = 5;
// note: not all levels are used
constexpr int32_t NUM_LEVELS = 26;
// levels have fewer than 13 checkpoints
// the 13 is an upper bound from the engine, used in the array of stats values
constexpr int32_t NUM_CHECKPOINTS_PER_LEVEL = 13;
constexpr int32_t MAX_CHALLENGES = 278;

using TI8 = Primitive<int8_t>;
using TI16 = Primitive<int16_t>;
using TI32 = Primitive<int32_t>;
using TI64 = Primitive<int64_t>;
using TF32 = Primitive<float>;

struct String {
    int32_t length;
    char text[40];
};

using TString = Struct<
    String,
    Fields<
        Field<TI32, &String::length>,
        Field<Ref<Primitive<char[40]>>, &String::text>>>;

struct LevelData {
    int32_t level;
};

using TLevelData
    = Struct<LevelData, Fields<Pad<0x08>, Field<TI32, &LevelData::level>>>;

struct CheckpointData {
    int32_t best_raw_score;
};

using TCheckpointData = Struct<
    CheckpointData,
    Fields<Pad<0x38>, Field<TI32, &CheckpointData::best_raw_score>>>;

struct CheckpointNode {
    std::optional<CheckpointData> data;
};

using TCheckpointNode = Struct<
    CheckpointNode,
    Fields<
        Pad<0x04>,
        Field<NullableRef<TCheckpointData>, &CheckpointNode::data>>>;

struct CheckpointInfo {
    std::vector<CheckpointNode> nodes;
};

using TCheckpointInfo = Struct<
    CheckpointInfo,
    Fields<
        Pad<0x10>,
        Field<Vector<TCheckpointNode, 0x1000>, &CheckpointInfo::nodes>>>;

struct GameDataLevelInfo {
    LevelData level_data;
    std::optional<CheckpointInfo> checkpoint_info;
};

using TGameDataLevelInfo = Struct<
    GameDataLevelInfo,
    Fields<
        Pad<0x04>,
        Field<Ref<TLevelData>, &GameDataLevelInfo::level_data>,
        Pad<0x04>,
        Field<
            NullableRef<TCheckpointInfo>,
            &GameDataLevelInfo::checkpoint_info>,
        Pad<0xA0>>>;

struct GameData {
    std::vector<GameDataLevelInfo> level_infos;
};

using TGameData = Struct<
    GameData,
    Fields<
        Pad<0x20>,
        Field<Vector<TGameDataLevelInfo, 0x1000>, &GameData::level_infos>>>;

struct LevelManager {
    String scene;
    int32_t game_mode;
    int32_t checkpoint_index;
};

using TLevelManager = Struct<
    LevelManager,
    Fields<
        Pad<0x04>,
        Field<TString, &LevelManager::scene>,
        Field<TI32, &LevelManager::game_mode>,
        Pad<0x10>,
        Field<TI32, &LevelManager::checkpoint_index>>>;

struct Checkpoint {
    int32_t key;
    int32_t _unknown;
};

using TCheckpoint = Struct<
    Checkpoint,
    Fields<Field<TI32, &Checkpoint::key>, Field<TI32, &Checkpoint::_unknown>>>;

struct Checkpoints {
    std::vector<Checkpoint> checkpoint;
    int32_t current_key;
};

using TCheckpoints = Struct<
    Checkpoints,
    Fields<
        Pad<0x0C>,
        Field<Vector<TCheckpoint, NUM_CHECKPOINTS_PER_LEVEL>, &Checkpoints::checkpoint>,
        Pad<0x3C>,
        Field<TI32, &Checkpoints::current_key>>>;

struct CheckpointsManager {
    std::optional<Checkpoints> checkpoints;
};

using TCheckpointsManager = Struct<
    CheckpointsManager,
    Fields<
        Pad<0x28>,
        Field<NullableRef<TCheckpoints>, &CheckpointsManager::checkpoints>>>;

struct TimeManager {
    int64_t ticks_per_second;
    int64_t last_time_ticks;
    int64_t game_time;
    int64_t game_time_previous;
    int64_t game_time_delta;
    int64_t real_time;
    int64_t real_time_previous;
    int64_t real_time_delta;
    float game_time_multiplier;
    float debug_time_multiplier;
    int64_t frame_wait;
    int64_t frame_step;
    int64_t frame_remain;
    int32_t paused;
    int32_t frame_count;
};

using TTimeManager = Struct<
    TimeManager,
    Fields<
        Pad<0x08>,
        Field<TI64, &TimeManager::ticks_per_second>,
        Field<TI64, &TimeManager::last_time_ticks>,
        Field<TI64, &TimeManager::game_time>,
        Field<TI64, &TimeManager::game_time_previous>,
        Field<TI64, &TimeManager::game_time_delta>,
        Field<TI64, &TimeManager::real_time>,
        Field<TI64, &TimeManager::real_time_previous>,
        Field<TI64, &TimeManager::real_time_delta>,
        Field<TF32, &TimeManager::game_time_multiplier>,
        Field<TF32, &TimeManager::debug_time_multiplier>,
        Field<TI64, &TimeManager::frame_wait>,
        Field<TI64, &TimeManager::frame_step>,
        Field<TI64, &TimeManager::frame_remain>,
        Field<TI32, &TimeManager::paused>,
        Field<TI32, &TimeManager::frame_count>>>;

struct StatsScoringData {
    String title;
    float _unused;
    int32_t index;
    int32_t multiplier;
};

using TStatsScoringData = Struct<
    StatsScoringData,
    Fields<
        Pad<0x08>,
        Field<TString, &StatsScoringData::title>,
        Pad<0x18>,
        Field<TF32, &StatsScoringData::_unused>,
        Pad<0x08>,
        Field<TI32, &StatsScoringData::index>,
        Pad<0x04>,
        Field<TI32, &StatsScoringData::multiplier>>>;

struct StatsScoring {
    StatsScoringData data;
};

using TStatsScoring = Struct<
    StatsScoring,
    Fields<Pad<0x04>, Field<Ref<TStatsScoringData>, &StatsScoring::data>>>;

struct PlaystyleConditionData {
    String title;
    int32_t index;
    int32_t threshold;
};

using TPlaystyleConditionData = Struct<
    PlaystyleConditionData,
    Fields<
        Pad<0x08>,
        Field<TString, &PlaystyleConditionData::title>,
        Pad<0x24>,
        Field<TI32, &PlaystyleConditionData::index>,
        Field<TI32, &PlaystyleConditionData::threshold>>>;

struct PlaystyleCondition {
    PlaystyleConditionData data;
};

using TPlaystyleCondition = Struct<
    PlaystyleCondition,
    Fields<
        Pad<0x04>,
        Field<Ref<TPlaystyleConditionData>, &PlaystyleCondition::data>>>;

struct StatsPlaystyleData {
    std::vector<PlaystyleCondition> condition_min;
    std::vector<PlaystyleCondition> condition_max;
    String title;
    int8_t is_unlockable;
    int32_t priority;
    int32_t percentage_min;
    int32_t percentage_max;
    int8_t is_achieved;
};

using TStatsPlaystyleData = Struct<
    StatsPlaystyleData,
    Fields<
        Pad<0x08>,
        Field<
            Vector<TPlaystyleCondition, 0x1000>,
            &StatsPlaystyleData::condition_min>,
        Pad<0x04>,
        Field<
            Vector<TPlaystyleCondition, 0x1000>,
            &StatsPlaystyleData::condition_max>,
        Pad<0x04>,
        Field<TString, &StatsPlaystyleData::title>,
        Pad<0x0C>,
        Field<TI8, &StatsPlaystyleData::is_unlockable>,
        Pad<0x03>,
        Field<TI32, &StatsPlaystyleData::priority>,
        Field<TI32, &StatsPlaystyleData::percentage_min>,
        Field<TI32, &StatsPlaystyleData::percentage_max>,
        Pad<0x0C>,
        Field<TI8, &StatsPlaystyleData::is_achieved>,
        Pad<0x03>>>;

struct StatsPlaystyle {
    StatsPlaystyleData data;
};

using TStatsPlaystyle = Struct<
    StatsPlaystyle,
    Fields<Pad<0x04>, Field<Ref<TStatsPlaystyleData>, &StatsPlaystyle::data>>>;

struct StatsDifficulties {
    std::array<float, 5> scales;
};

using TStatsDifficulties = Struct<
    StatsDifficulties,
    Fields<Pad<0x08>, Field<Array<TF32, 5>, &StatsDifficulties::scales>>>;

using StatsValues = std::array<std::array<std::array<int16_t, 100>, 13>, 26>;
using AchievedPlaystyles = std::array<int8_t, 100>;

struct StatsManager {
    std::vector<StatsScoring> scorings;
    std::vector<StatsPlaystyle> playstyles;
    StatsValues values;
    AchievedPlaystyles achieved_playstyles;
    int32_t last_achieved_playstyle;
    int32_t score;
    std::optional<StatsDifficulties> difficulties;
};

using TStatsManager = Struct<
    StatsManager,
    Fields<
        Pad<0x04>,
        Field<Vector<TStatsScoring, 0x1000>, &StatsManager::scorings>,
        Pad<0x04>,
        Field<Vector<TStatsPlaystyle, 0x1000>, &StatsManager::playstyles>,
        Pad<0x10>,
        Field<Ref<Primitive<StatsValues>>, &StatsManager::values>,
        Pad<0x08>,
        Field<
            Primitive<AchievedPlaystyles>,
            &StatsManager::achieved_playstyles>,
        Pad<0x2C>,
        Field<TI32, &StatsManager::last_achieved_playstyle>,
        Pad<0x18>,
        Field<TI32, &StatsManager::score>,
        Pad<0x18>,
        Field<NullableRef<TStatsDifficulties>, &StatsManager::difficulties>>>;

struct ChallengeData {
    int8_t completed;
};

using TChallengeData = Struct<
    ChallengeData,
    Fields<Pad<0x98>, Field<TI8, &ChallengeData::completed>>>;

struct ChallengeNode {
    uint32_t next_node;
    std::optional<ChallengeData> data;
};

using TChallengeNode = Struct<
    ChallengeNode,
    Fields<
        Field<RawAddr<uint32_t>, &ChallengeNode::next_node>,
        Pad<0x08>,
        Field<NullableRef<TChallengeData>, &ChallengeNode::data>>>;

struct ChallengeManager {
    std::vector<ChallengeNode> challenges;
};

using TChallengeManager = Struct<
    ChallengeManager,
    Fields<
        Pad<0x08>,
        Field<
            CircularList<TChallengeNode, &ChallengeNode::next_node, 0x1000>,
            &ChallengeManager::challenges>>>;

struct Game {
    int32_t difficulty;
    StatsManager stats_manager;
    ChallengeManager challenge_manager;
    GameData game_data;
    LevelManager level_manager;
    int32_t level;
    CheckpointsManager checkpoints_manager;
    TimeManager time_manager;
};

using TGame = Struct<
    Game,
    Fields<
        Seek<0xD58C60 + 0x10 + 0x94>,
        Field<TI32, &Game::difficulty>,
        Seek<0xD61710>,
        Field<TStatsManager, &Game::stats_manager>,
        Seek<0xD617C0>,
        Field<TChallengeManager, &Game::challenge_manager>,
        Seek<0xE212E0>,
        Field<TGameData, &Game::game_data>,
        Seek<0xE21310>,
        Field<TLevelManager, &Game::level_manager>,
        Seek<0xE21394>,
        Field<TI32, &Game::level>,
        Seek<0xE21580>,
        Field<TCheckpointsManager, &Game::checkpoints_manager>,
        Seek<0xE24730>,
        Field<TTimeManager, &Game::time_manager>>>;

}  // namespace hitman_absolution::descriptors