#pragma once

#include <array>
#include <cstdint>
#include <mempeep/descriptors.hpp>
#include <optional>
#include <string>
#include <vector>

using namespace mempeep;

namespace hitman_absolution::structs {

struct String {
    int32_t length;
    std::string text;
};

using TString = Struct<
    String,
    Fields<
        Field<Int32, &String::length>,
        Field<Ref<ZString<0x40>>, &String::text>>>;

struct StatsScoringData {
    String title;
    float _unused;
    int32_t index;
    int32_t multiplier;
};

using TStatsScoringData = Struct<
    StatsScoringData,
    Fields<
        Skip<0x8>,
        Field<TString, &StatsScoringData::title>,
        Skip<0x18>,
        Field<Float, &StatsScoringData::_unused>,
        Skip<0x8>,
        Field<Int32, &StatsScoringData::index>,
        Skip<0x4>,
        Field<Int32, &StatsScoringData::multiplier>>>;

struct StatsScoring {
    StatsScoringData data;
};

using TStatsScoring = Struct<
    StatsScoring,
    Fields<Skip<0x4>, Field<Ref<TStatsScoringData>, &StatsScoring::data>>>;

struct PlaystyleConditionData {
    String title;
    int32_t index;
    int32_t threshold;
};

using TPlaystyleConditionData = Struct<
    PlaystyleConditionData,
    Fields<
        Skip<0x8>,
        Field<TString, &PlaystyleConditionData::title>,
        Skip<0x24>,
        Field<Int32, &PlaystyleConditionData::index>,
        Field<Int32, &PlaystyleConditionData::threshold>>>;

struct PlaystyleCondition {
    PlaystyleConditionData data;
};

using TPlaystyleCondition = Struct<
    PlaystyleCondition,
    Fields<
        Skip<0x4>,
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
        Skip<0x8>,
        Field<
            Vector<TPlaystyleCondition, 0xa>,
            &StatsPlaystyleData::condition_min>,
        Skip<0x4>,
        Field<
            Vector<TPlaystyleCondition, 0xa>,
            &StatsPlaystyleData::condition_max>,
        Skip<0x4>,
        Field<TString, &StatsPlaystyleData::title>,
        Skip<0xc>,
        Field<Int8, &StatsPlaystyleData::is_unlockable>,
        Skip<0x3>,
        Field<Int32, &StatsPlaystyleData::priority>,
        Field<Int32, &StatsPlaystyleData::percentage_min>,
        Field<Int32, &StatsPlaystyleData::percentage_max>,
        Skip<0xc>,
        Field<Int8, &StatsPlaystyleData::is_achieved>,
        Skip<0x3>>>;

struct StatsPlaystyle {
    StatsPlaystyleData data;
};

using TStatsPlaystyle = Struct<
    StatsPlaystyle,
    Fields<Skip<0x4>, Field<Ref<TStatsPlaystyleData>, &StatsPlaystyle::data>>>;

struct StatsDifficulties {
    std::array<float, 0x5> scales;
};

using TStatsDifficulties = Struct<
    StatsDifficulties,
    Fields<
        Skip<0x8>,
        Field<Primitive<std::array<float, 0x5>>, &StatsDifficulties::scales>>>;

struct StatsManager {
    std::vector<StatsScoring> scorings;
    std::vector<StatsPlaystyle> playstyles;
    std::array<std::array<std::array<int16_t, 0x64>, 0xd>, 0x1a> values;
    std::array<int8_t, 0x64> achieved_playstyles;
    int32_t last_achieved_playstyle;
    int32_t score;
    std::optional<StatsDifficulties> difficulties;
};

using TStatsManager = Struct<
    StatsManager,
    Fields<
        Skip<0x4>,
        Field<Vector<TStatsScoring, 0x64>, &StatsManager::scorings>,
        Skip<0x4>,
        Field<Vector<TStatsPlaystyle, 0x1a>, &StatsManager::playstyles>,
        Skip<0x10>,
        Field<
            Ref<Primitive<
                std::array<std::array<std::array<int16_t, 0x64>, 0xd>, 0x1a>>>,
            &StatsManager::values>,
        Skip<0x8>,
        Field<
            Ref<Primitive<std::array<int8_t, 0x64>>>,
            &StatsManager::achieved_playstyles>,
        Skip<0x2c>,
        Field<Int32, &StatsManager::last_achieved_playstyle>,
        Skip<0x18>,
        Field<Int32, &StatsManager::score>,
        Skip<0x18>,
        Field<NullableRef<TStatsDifficulties>, &StatsManager::difficulties>>>;

struct ChallengeData {
    int8_t completed;
};

using TChallengeData = Struct<
    ChallengeData,
    Fields<Skip<0x98>, Field<Int8, &ChallengeData::completed>>>;

struct ChallengeNode {
    uintptr_t next_node;
    std::optional<ChallengeData> data;
};

using TChallengeNode = Struct<
    ChallengeNode,
    Fields<
        Field<RawAddr<uintptr_t>, &ChallengeNode::next_node>,
        Skip<0x8>,
        Field<NullableRef<TChallengeData>, &ChallengeNode::data>>>;

struct ChallengeManager {
    std::vector<ChallengeNode> challenges;
};

using TChallengeManager = Struct<
    ChallengeManager,
    Fields<
        Skip<0x8>,
        Field<
            CircularList<TChallengeNode, &ChallengeNode::next_node, 0x12c>,
            &ChallengeManager::challenges>>>;

struct LevelData {
    int32_t level;
};

using TLevelData
    = Struct<LevelData, Fields<Skip<0x8>, Field<Int32, &LevelData::level>>>;

struct CheckpointData {
    int32_t best_raw_score;
};

using TCheckpointData = Struct<
    CheckpointData,
    Fields<Skip<0x38>, Field<Int32, &CheckpointData::best_raw_score>>>;

struct CheckpointNode {
    std::optional<CheckpointData> data;
};

using TCheckpointNode = Struct<
    CheckpointNode,
    Fields<
        Skip<0x4>,
        Field<NullableRef<TCheckpointData>, &CheckpointNode::data>>>;

struct CheckpointInfo {
    std::vector<CheckpointNode> nodes;
};

using TCheckpointInfo = Struct<
    CheckpointInfo,
    Fields<
        Skip<0x10>,
        Field<Vector<TCheckpointNode, 0xd>, &CheckpointInfo::nodes>>>;

struct GameDataLevelInfo {
    LevelData level_data;
    std::optional<CheckpointInfo> checkpoint_info;
};

using TGameDataLevelInfo = Struct<
    GameDataLevelInfo,
    Fields<
        Skip<0x4>,
        Field<Ref<TLevelData>, &GameDataLevelInfo::level_data>,
        Skip<0x4>,
        Field<
            NullableRef<TCheckpointInfo>,
            &GameDataLevelInfo::checkpoint_info>,
        Skip<0xa0>>>;

struct GameData {
    std::vector<GameDataLevelInfo> level_infos;
};

using TGameData = Struct<
    GameData,
    Fields<
        Skip<0x20>,
        Field<Vector<TGameDataLevelInfo, 0x1a>, &GameData::level_infos>>>;

struct LevelManager {
    String scene;
    int32_t game_mode;
    int32_t checkpoint_index;
};

using TLevelManager = Struct<
    LevelManager,
    Fields<
        Skip<0x4>,
        Field<TString, &LevelManager::scene>,
        Field<Int32, &LevelManager::game_mode>,
        Skip<0x10>,
        Field<Int32, &LevelManager::checkpoint_index>>>;

struct Checkpoint {
    int32_t key;
    int32_t _unknown;
};

using TCheckpoint = Struct<
    Checkpoint,
    Fields<
        Field<Int32, &Checkpoint::key>,
        Field<Int32, &Checkpoint::_unknown>>>;

struct Checkpoints {
    std::vector<Checkpoint> checkpoint;
    int32_t current_key;
};

using TCheckpoints = Struct<
    Checkpoints,
    Fields<
        Skip<0xc>,
        Field<Vector<TCheckpoint, 0xd>, &Checkpoints::checkpoint>,
        Skip<0x3c>,
        Field<Int32, &Checkpoints::current_key>>>;

struct CheckpointsManager {
    std::optional<Checkpoints> checkpoints;
};

using TCheckpointsManager = Struct<
    CheckpointsManager,
    Fields<
        Skip<0x28>,
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
        Skip<0x8>,
        Field<Int64, &TimeManager::ticks_per_second>,
        Field<Int64, &TimeManager::last_time_ticks>,
        Field<Int64, &TimeManager::game_time>,
        Field<Int64, &TimeManager::game_time_previous>,
        Field<Int64, &TimeManager::game_time_delta>,
        Field<Int64, &TimeManager::real_time>,
        Field<Int64, &TimeManager::real_time_previous>,
        Field<Int64, &TimeManager::real_time_delta>,
        Field<Float, &TimeManager::game_time_multiplier>,
        Field<Float, &TimeManager::debug_time_multiplier>,
        Field<Int64, &TimeManager::frame_wait>,
        Field<Int64, &TimeManager::frame_step>,
        Field<Int64, &TimeManager::frame_remain>,
        Field<Int32, &TimeManager::paused>,
        Field<Int32, &TimeManager::frame_count>>>;

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
        Seek<0xd58d04>,
        Field<Bounded<Int32, 0, 4>, &Game::difficulty>,
        Seek<0xd61710>,
        Field<TStatsManager, &Game::stats_manager>,
        Seek<0xd617c0>,
        Field<TChallengeManager, &Game::challenge_manager>,
        Seek<0xe212e0>,
        Field<TGameData, &Game::game_data>,
        Seek<0xe21310>,
        Field<TLevelManager, &Game::level_manager>,
        Seek<0xe21394>,
        Field<Bounded<Int32, -1, 25>, &Game::level>,
        Seek<0xe21580>,
        Field<TCheckpointsManager, &Game::checkpoints_manager>,
        Seek<0xe24730>,
        Field<TTimeManager, &Game::time_manager>>>;

}  // namespace hitman_absolution::structs
