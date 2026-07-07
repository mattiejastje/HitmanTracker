#pragma once

#include <cstdint>
#include <mempeep/descriptors.hpp>
#include <optional>
#include <string>

using namespace mempeep;

namespace hitman_contracts::structs {

struct PropertyType {
    uint32_t fourcc;
    int32_t fourcc_length;
    int32_t index;
    int32_t size;
    int32_t type_flags;
};

struct Property {
    int32_t key_length;
    PropertyType type;
    int32_t size;
    std::string key;
};

using TProperty = Struct<
    Property,
    Fields<
        Field<Int32, &Property::key_length>,
        Field<Ref<Primitive<PropertyType>>, &Property::type>,
        Field<Int32, &Property::size>,
        Field<ZString<0x40>, &Property::key>>>;

struct PropertyManagerRecord {
    int32_t record_size;
    int8_t is_active;
    RemoteValue<TProperty, uint32_t> property;
};

using TPropertyManagerRecord = Struct<
    PropertyManagerRecord,
    Fields<
        Field<Int32, &PropertyManagerRecord::record_size>,
        Field<Int8, &PropertyManagerRecord::is_active>,
        Field<
            RemoteAddr<TProperty, uint32_t>,
            &PropertyManagerRecord::property>>>;

struct SmallString {
    std::string text;
};

using TSmallString = Struct<
    SmallString,
    Fields<Field<Ref<ZString<0x100>>, &SmallString::text>, Skip<0x7c>>>;

struct SceneManager {
    SmallString scene_name;
};

using TSceneManager = Struct<
    SceneManager,
    Fields<Seek<0xbcd>, Field<TSmallString, &SceneManager::scene_name>>>;

struct Engine {
    int32_t game_ticks;
    SceneManager scene_manager;
};

using TEngine = Struct<
    Engine,
    Fields<
        Seek<0x38>,
        Field<Int32, &Engine::game_ticks>,
        Seek<0xa5>,
        Field<Ref<TSceneManager>, &Engine::scene_manager>>>;

struct PropertyManager {
    uint32_t vtable;
    int32_t data_capacity;
    uint32_t data;
    int32_t data_used;
};

struct PlayerData {
    int32_t shots_fired;
};

using TPlayerData = Struct<
    PlayerData,
    Fields<Seek<0x13db>, Field<Int32, &PlayerData::shots_fired>>>;

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
    PropertyManager property_manager;
    Player player;
};

using THitmanContractsSteam = Struct<
    HitmanContracts,
    Fields<
        Seek<0x39457c>,
        Field<Ref<TEngine>, &HitmanContracts::engine>,
        Seek<0x39459c>,
        Field<
            Ref<Primitive<PropertyManager>>,
            &HitmanContracts::property_manager>,
        Seek<0x3947a8>,
        Field<TPlayer, &HitmanContracts::player>>>;

using THitmanContractsGOG = Struct<
    HitmanContracts,
    Fields<
        Seek<0x393ddc>,
        Field<Ref<TEngine>, &HitmanContracts::engine>,
        Seek<0x393dfc>,
        Field<
            Ref<Primitive<PropertyManager>>,
            &HitmanContracts::property_manager>,
        Seek<0x394008>,
        Field<TPlayer, &HitmanContracts::player>>>;

}  // namespace hitman_contracts::structs
