#pragma once

#include <cstdint>
#include <mempeep/descriptors.hpp>
#include <optional>
#include <string>

using namespace mempeep;

namespace hitman_contracts::structs {

struct SceneEntityManagerUnk08 {
    int32_t mask;
};

using TSceneEntityManagerUnk08 = Struct<
    SceneEntityManagerUnk08,
    Fields<Seek<0x10>, Field<Int32, &SceneEntityManagerUnk08::mask>>>;

struct SceneEntityManager {
    SceneEntityManagerUnk08 unk_08;
};

using TSceneEntityManager = Struct<
    SceneEntityManager,
    Fields<
        Seek<0x8>,
        Field<Ref<TSceneEntityManagerUnk08>, &SceneEntityManager::unk_08>>>;

struct SmallString {
    std::string text;
};

using TSmallString = Struct<
    SmallString,
    Fields<Field<ZString<0x100>, &SmallString::text>, Skip<0x7c>>>;

struct SceneManager {
    std::optional<SceneEntityManager> entity_manager;
    int8_t pause_flag_1;
    int8_t pause_flag_2;
    SmallString scene_name;
    int32_t unk_6d39;
};

using TSceneManager = Struct<
    SceneManager,
    Fields<
        Skip<0x4>,
        Field<NullableRef<TSceneEntityManager>, &SceneManager::entity_manager>,
        Seek<0xbb0>,
        Field<Int8, &SceneManager::pause_flag_1>,
        Field<Int8, &SceneManager::pause_flag_2>,
        Seek<0xbcd>,
        Field<TSmallString, &SceneManager::scene_name>,
        Seek<0x6d39>,
        Field<Int32, &SceneManager::unk_6d39>>>;

struct Engine {
    float game_time;
    int32_t game_ticks_copy;
    int32_t game_ticks;
    int32_t game_ticks_previous;
    float frame_time;
    int32_t pause_ticks_offset;
    SceneManager scene_manager;
    float game_time_update_interval;
};

using TEngine = Struct<
    Engine,
    Fields<
        Seek<0x24>,
        Field<Float, &Engine::game_time>,
        Seek<0x30>,
        Field<Int32, &Engine::game_ticks_copy>,
        Skip<0x4>,
        Field<Int32, &Engine::game_ticks>,
        Field<Int32, &Engine::game_ticks_previous>,
        Field<Float, &Engine::frame_time>,
        Field<Int32, &Engine::pause_ticks_offset>,
        Seek<0xa5>,
        Field<Ref<TSceneManager>, &Engine::scene_manager>,
        Seek<0x80d>,
        Field<Float, &Engine::game_time_update_interval>>>;

struct PlayerData {
    int8_t unk_flag_e59;
    int32_t shots_fired;
};

using TPlayerData = Struct<
    PlayerData,
    Fields<
        Seek<0xe59>,
        Field<Int8, &PlayerData::unk_flag_e59>,
        Seek<0x13db>,
        Field<Int32, &PlayerData::shots_fired>>>;

struct PlayerStats {
    float aggression;
    int32_t headshots;
    int32_t enemies_wounded;
    int32_t enemies_killed;
    int32_t innocents_wounded;
    int32_t innocents_killed;
    int32_t alerts;
    int32_t close_encounters;
};

using TPlayerStats = Struct<
    PlayerStats,
    Fields<
        Seek<0xb13>,
        Field<Float, &PlayerStats::aggression>,
        Field<Int32, &PlayerStats::headshots>,
        Field<Int32, &PlayerStats::enemies_wounded>,
        Field<Int32, &PlayerStats::enemies_killed>,
        Field<Int32, &PlayerStats::innocents_wounded>,
        Field<Int32, &PlayerStats::innocents_killed>,
        Field<Int32, &PlayerStats::alerts>,
        Field<Int32, &PlayerStats::close_encounters>>>;

struct Player {
    std::optional<PlayerData> data;
    std::optional<PlayerStats> stats;
};

using TPlayer = Struct<
    Player,
    Fields<
        Skip<0x8>,
        Field<NullableRef<TPlayerData>, &Player::data>,
        Seek<0x18>,
        Field<NullableRef<TPlayerStats>, &Player::stats>>>;

struct HitmanContracts {
    Engine engine;
    Player player;
};

using THitmanContracts = Struct<
    HitmanContracts,
    Fields<
        Seek<0x39457c>,
        Field<Ref<TEngine>, &HitmanContracts::engine>,
        Seek<0x3947a8>,
        Field<TPlayer, &HitmanContracts::player>>>;

}  // namespace hitman_contracts::structs
