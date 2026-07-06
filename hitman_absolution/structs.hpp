#pragma once

#include <array>
#include <cstdint>
#include <mempeep/descriptors.hpp>
#include <optional>
#include <string>
#include <vector>

using namespace mempeep;

namespace hitman_absolution::structs {

struct GlobalData {
    uint8_t _pad0[0x94];
    int32_t difficulty;
};

struct StatsManager {
    RemoteValue<
        Primitive<std::array<std::array<std::array<int16_t, 0x64>, 0xd>, 0x1a>>,
        uint32_t>
        values;
};

using TStatsManager = Struct<
    StatsManager,
    Fields<
        Seek<0x28>,
        Field<
            Ref<RemoteAddr<
                Primitive<std::array<
                    std::array<std::array<int16_t, 0x64>, 0xd>,
                    0x1a>>,
                uint32_t>>,
            &StatsManager::values>>>;

struct ChallengeData {
    int8_t completed;
};

using TChallengeData = Struct<
    ChallengeData,
    Fields<Seek<0x98>, Field<Int8, &ChallengeData::completed>>>;

struct ChallengeNode {
    uint32_t next_node;
    std::optional<ChallengeData> data;
};

using TChallengeNode = Struct<
    ChallengeNode,
    Fields<
        Field<RawAddr<uint32_t>, &ChallengeNode::next_node>,
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
            List<
                TChallengeNode,
                &ChallengeNode::next_node,
                ListKind::CIRCULAR,
                0x12c>,
            &ChallengeManager::challenges>>>;

struct EventManager {
    int32_t npcs_killed;
};

using TEventManager = Struct<
    EventManager,
    Fields<Seek<0xfc>, Field<Int32, &EventManager::npcs_killed>>>;

struct LevelData {
    uint8_t _pad0[0x8];
    int32_t level;
};

struct CheckpointData {
    uint8_t _pad0[0x38];
    int32_t shadow_raw_score_threshold;
};

struct CheckpointNode {
    std::optional<CheckpointData> data;
};

using TCheckpointNode = Struct<
    CheckpointNode,
    Fields<
        Skip<0x4>,
        Field<NullableRef<Primitive<CheckpointData>>, &CheckpointNode::data>>>;

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
        Field<Ref<Primitive<LevelData>>, &GameDataLevelInfo::level_data>,
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

struct String {
    int32_t length;
    std::string text;
};

using TString = Struct<
    String,
    Fields<
        Field<Int32, &String::length>,
        Field<Ref<ZString<0x40>>, &String::text>>>;

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

struct Checkpoints {
    std::vector<Checkpoint> checkpoint;
    int32_t current_key;
};

using TCheckpoints = Struct<
    Checkpoints,
    Fields<
        Skip<0xc>,
        Field<Vector<Primitive<Checkpoint>, 0xd>, &Checkpoints::checkpoint>,
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
    uint8_t _pad0[0x8];
    int64_t ticks_per_second;
    int64_t last_time_ticks;
    int64_t game_time;
};

struct MovieInfoBuffer {
    int32_t is_allocated;
    uint32_t buffer;
    int32_t unk_maybe_width;
};

struct MovieInfo {
    int32_t num_allocated_planes;
    int32_t width;
    int32_t height;
    int32_t chroma_width;
    int32_t chroma_height;
    int32_t unk_14;
    std::array<std::array<MovieInfoBuffer, 0x4>, 0x3> planes;
    int32_t unk_a8;
};

struct MovieManagerData {
    uint8_t _pad0[0x8];
    MovieInfo info;
};

struct MovieManager {
    MovieManagerData data;
};

using TMovieManager = Struct<
    MovieManager,
    Fields<
        Seek<0x84>,
        Field<Ref<Primitive<MovieManagerData>>, &MovieManager::data>>>;

struct Game {
    GlobalData global_data;
    StatsManager stats_manager;
    ChallengeManager challenge_manager;
    EventManager event_manager;
    GameData game_data;
    LevelManager level_manager;
    int32_t level;
    CheckpointsManager checkpoints_manager;
    TimeManager time_manager;
    MovieManager movie_manager;
};

using TGameSteam = Struct<
    Game,
    Fields<
        Seek<0xd58c70>,
        Field<Primitive<GlobalData>, &Game::global_data>,
        Seek<0xd61710>,
        Field<TStatsManager, &Game::stats_manager>,
        Seek<0xd617c0>,
        Field<TChallengeManager, &Game::challenge_manager>,
        Seek<0xe20e40>,
        Field<TEventManager, &Game::event_manager>,
        Seek<0xe212e0>,
        Field<TGameData, &Game::game_data>,
        Seek<0xe21310>,
        Field<TLevelManager, &Game::level_manager>,
        Seek<0xe21394>,
        Field<Bounded<Int32, -1, 25>, &Game::level>,
        Seek<0xe21580>,
        Field<TCheckpointsManager, &Game::checkpoints_manager>,
        Seek<0xe24730>,
        Field<Primitive<TimeManager>, &Game::time_manager>,
        Seek<0xe31b80>,
        Field<TMovieManager, &Game::movie_manager>>>;

using TGameGOG = Struct<
    Game,
    Fields<
        Seek<0xca0850>,
        Field<Primitive<GlobalData>, &Game::global_data>,
        Seek<0xca92d0>,
        Field<TStatsManager, &Game::stats_manager>,
        Seek<0xca9380>,
        Field<TChallengeManager, &Game::challenge_manager>,
        Seek<0xd68a00>,
        Field<TEventManager, &Game::event_manager>,
        Seek<0xd68ea0>,
        Field<TGameData, &Game::game_data>,
        Seek<0xd68ed0>,
        Field<TLevelManager, &Game::level_manager>,
        Seek<0xd68f54>,
        Field<Bounded<Int32, -1, 25>, &Game::level>,
        Seek<0xd69140>,
        Field<TCheckpointsManager, &Game::checkpoints_manager>,
        Seek<0xc88580>,
        Field<Primitive<TimeManager>, &Game::time_manager>,
        Seek<0xc87c00>,
        Field<TMovieManager, &Game::movie_manager>>>;

}  // namespace hitman_absolution::structs
