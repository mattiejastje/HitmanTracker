#pragma once

#include <cstdint>
#include <mempeep/descriptors.hpp>
#include <optional>
#include <string>

using namespace mempeep;

namespace hitman2_silent_assassin::structs {

struct LevelControl {
    uint8_t _pad0[0x208];
    int32_t headshots;
    int32_t enemies_wounded;
    int32_t enemies_killed;
    int32_t innocents_wounded;
    int32_t innocents_killed;
    int32_t alerts;
    int32_t close_encounters;
};

struct PlayerEntity {
    uint32_t gref;
};

using TPlayerEntity = Struct<
    PlayerEntity,
    Fields<Seek<0x48>, Field<UInt32, &PlayerEntity::gref>>>;

struct PlayerData {
    int32_t player_gref;
    int32_t shots_fired;
};

using TPlayerData = Struct<
    PlayerData,
    Fields<
        Seek<0xb6d>,
        Field<Int32, &PlayerData::player_gref>,
        Seek<0x11c7>,
        Field<Int32, &PlayerData::shots_fired>,
        Seek<0x1263>>>;

struct Player {
    PlayerData data;
};

using TPlayer = Struct<
    Player,
    Fields<Seek<0x54>, Field<Ref<TPlayerData>, &Player::data>>>;

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

struct EntityManager {
    std::optional<
        RemoteValue<Primitive<std::array<int32_t, 0x40000>>, uint32_t>>
        versions;
    std::optional<
        RemoteValue<Primitive<std::array<uint32_t, 0x40000>>, uint32_t>>
        entities;
};

using TEntityManager = Struct<
    EntityManager,
    Fields<
        Seek<0x24>,
        Field<
            NullableRef<
                RemoteAddr<Primitive<std::array<int32_t, 0x40000>>, uint32_t>>,
            &EntityManager::versions>,
        Field<
            NullableRef<
                RemoteAddr<Primitive<std::array<uint32_t, 0x40000>>, uint32_t>>,
            &EntityManager::entities>>>;

struct GRefManagerPool {
    uint8_t _pad0[0x4];
    uint32_t base;
};

struct GRefManager {
    uint32_t vtable;
    GRefManagerPool pool;
    int32_t is_allocated;
    int32_t pool_size;
    uint32_t slots;
};

using TGRefManager = Struct<
    GRefManager,
    Fields<
        Field<RawAddr<uint32_t>, &GRefManager::vtable>,
        Seek<0x14>,
        Field<Ref<Primitive<GRefManagerPool>>, &GRefManager::pool>,
        Seek<0x24>,
        Field<Int32, &GRefManager::is_allocated>,
        Field<Int32, &GRefManager::pool_size>,
        Seek<0x5d>,
        Field<RawAddr<uint32_t>, &GRefManager::slots>>>;

struct SmallString {
    std::string text;
};

using TSmallString = Struct<
    SmallString,
    Fields<Field<Ref<ZString<0x100>>, &SmallString::text>, Skip<0x7c>>>;

struct SceneManager {
    std::optional<GRefManager> gref_manager;
    SmallString scene_name;
};

using TSceneManager = Struct<
    SceneManager,
    Fields<
        Skip<0x4>,
        Field<NullableRef<TGRefManager>, &SceneManager::gref_manager>,
        Seek<0xbb7>,
        Field<TSmallString, &SceneManager::scene_name>>>;

struct Engine {
    SceneManager scene_manager;
};

using TEngine = Struct<
    Engine,
    Fields<Seek<0x98>, Field<Ref<TSceneManager>, &Engine::scene_manager>>>;

struct PropertyManager {
    uint32_t vtable;
    int32_t data_capacity;
    uint32_t data;
    int32_t data_used;
};

struct Game {
    EntityManager entity_manager;
    Engine engine;
    PropertyManager property_manager;
};

using TGameSteam = Struct<
    Game,
    Fields<
        Seek<0x2a6c50>,
        Field<Ref<TEntityManager>, &Game::entity_manager>,
        Seek<0x2a6c5c>,
        Field<Ref<TEngine>, &Game::engine>,
        Seek<0x2a6c7c>,
        Field<Ref<Primitive<PropertyManager>>, &Game::property_manager>>>;

using TGameGOG = Struct<
    Game,
    Fields<
        Seek<0x2a8c58>,
        Field<Ref<TEntityManager>, &Game::entity_manager>,
        Seek<0x2a8c64>,
        Field<Ref<TEngine>, &Game::engine>,
        Seek<0x2a8c84>,
        Field<Ref<Primitive<PropertyManager>>, &Game::property_manager>>>;

}  // namespace hitman2_silent_assassin::structs
