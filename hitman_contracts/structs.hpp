#pragma once

#include <cstdint>
#include <mempeep/descriptors.hpp>
#include <optional>
#include <string>

using namespace mempeep;

namespace hitman_contracts::structs {

struct SmallString {
    std::string text;
};

using TSmallString = Struct<
    SmallString,
    Fields<Field<ZString<0x100>, &SmallString::text>, Skip<0x7c>>>;

struct SceneManager {
    int8_t pause_flag_1;
    int8_t pause_flag_2;
    SmallString scene_name;
    int32_t unk_6d39;
};

using TSceneManager = Struct<
    SceneManager,
    Fields<
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
    int32_t shots_fired;
};

using TPlayerData = Struct<
    PlayerData,
    Fields<Seek<0x13db>, Field<Int32, &PlayerData::shots_fired>>>;

struct PlayerStats {
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
        Seek<0xb17>,
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
    float seconds_per_tick;
    float ticks_per_second;
    Engine engine;
    uintptr_t player_ptr;
    Player player;
    uintptr_t player_data_copy;
};

using THitmanContracts = Struct<
    HitmanContracts,
    Fields<
        Seek<0x30e484>,
        Field<Float, &HitmanContracts::seconds_per_tick>,
        Seek<0x30e808>,
        Field<Float, &HitmanContracts::ticks_per_second>,
        Seek<0x39457c>,
        Field<Ref<TEngine>, &HitmanContracts::engine>,
        Seek<0x3945a4>,
        Field<RawAddr<uintptr_t>, &HitmanContracts::player_ptr>,
        Seek<0x3947a8>,
        Field<TPlayer, &HitmanContracts::player>,
        Seek<0x395718>,
        Field<RawAddr<uintptr_t>, &HitmanContracts::player_data_copy>>>;

}  // namespace hitman_contracts::structs
