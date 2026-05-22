#pragma once

#include <array>
#include <cstdint>
#include <mempeep/descriptors.hpp>
#include <optional>
#include <string>
#include <vector>

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
    std::array<std::array<float, 0x3>, 0x3> unk_matrix_00;
    int32_t unk_flag_3c;
    PlayerData data;
    int8_t unk_flag_58;
    int32_t unk_6c;
};

using TPlayer = Struct<
    Player,
    Fields<
        Field<
            Primitive<std::array<std::array<float, 0x3>, 0x3>>,
            &Player::unk_matrix_00>,
        Seek<0x3c>,
        Field<Int32, &Player::unk_flag_3c>,
        Seek<0x54>,
        Field<Ref<TPlayerData>, &Player::data>,
        Field<Int8, &Player::unk_flag_58>,
        Seek<0x6c>,
        Field<Int32, &Player::unk_6c>>>;

struct PropertyType {
    uint32_t fourcc;
    int32_t fourcc_length;
    int32_t unk_id;
    int32_t size;
    int32_t unk_elem_type;
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

struct SceneEntities {
    RemoteValue<Primitive<std::array<int32_t, 0x3e8>>, uint32_t> handles;
    uint32_t unk_08;
    int32_t unk_0c;
    int32_t num_handles;
};

using TSceneEntities = Struct<
    SceneEntities,
    Fields<
        Skip<0x4>,
        Field<
            Ref<RemoteAddr<Primitive<std::array<int32_t, 0x3e8>>, uint32_t>>,
            &SceneEntities::handles>,
        Field<RawAddr<uint32_t>, &SceneEntities::unk_08>,
        Field<Int32, &SceneEntities::unk_0c>,
        Field<Int32, &SceneEntities::num_handles>>>;

struct SmallString {
    std::string text;
};

using TSmallString = Struct<
    SmallString,
    Fields<Field<Ref<ZString<0x100>>, &SmallString::text>, Skip<0x7c>>>;

struct PropertyBlock {
    uint32_t prev_block;
    uint32_t next_block;
    int32_t num_properties;
    int32_t tombstone_marker;
    std::array<uint32_t, 0x20> properties;
};

struct SharedComContainer {
    uint32_t vtable;
    std::vector<PropertyBlock> blocks;
    uint32_t last_block;
    int32_t unk_flags;
    int32_t max_num_properties_per_block;
    int32_t num_properties_total;
    int32_t property_size;
};

using TSharedComContainer = Struct<
    SharedComContainer,
    Fields<
        Field<RawAddr<uint32_t>, &SharedComContainer::vtable>,
        Field<
            List<
                Primitive<PropertyBlock>,
                &PropertyBlock::next_block,
                ListKind::NULL_TERMINATED,
                0x1000>,
            &SharedComContainer::blocks>,
        Field<RawAddr<uint32_t>, &SharedComContainer::last_block>,
        Field<Int32, &SharedComContainer::unk_flags>,
        Field<
            Bounded<Int32, 32, 32>,
            &SharedComContainer::max_num_properties_per_block>,
        Field<Int32, &SharedComContainer::num_properties_total>,
        Field<Bounded<Int32, 1, 1>, &SharedComContainer::property_size>>>;

struct SharedCom {
    uint32_t vtable;
    SharedComContainer container;
};

using TSharedCom = Struct<
    SharedCom,
    Fields<
        Field<RawAddr<uint32_t>, &SharedCom::vtable>,
        Seek<0x4008>,
        Field<TSharedComContainer, &SharedCom::container>>>;

struct SceneManager {
    std::optional<GRefManager> gref_manager;
    std::optional<SceneEntities> entities;
    SmallString scene_name;
    SharedCom shared_com;
};

using TSceneManager = Struct<
    SceneManager,
    Fields<
        Skip<0x4>,
        Field<NullableRef<TGRefManager>, &SceneManager::gref_manager>,
        Seek<0xc4>,
        Field<NullableRef<TSceneEntities>, &SceneManager::entities>,
        Seek<0xbb7>,
        Field<TSmallString, &SceneManager::scene_name>,
        Seek<0x1c4b>,
        Field<TSharedCom, &SceneManager::shared_com>>>;

struct Engine {
    SceneManager scene_manager;
};

using TEngine = Struct<
    Engine,
    Fields<Seek<0x98>, Field<Ref<TSceneManager>, &Engine::scene_manager>>>;

struct Game {
    uint32_t engine_ptr;
    EntityManager entity_manager;
    Engine engine;
    std::string lethed;
    std::string current_level_name;
    int32_t shots_fired;
    int32_t close_encounters;
    int32_t headshots;
    int32_t alerts;
    int32_t enemies_killed;
    int32_t enemies_wounded;
    int32_t innocents_killed;
    int32_t innocents_wounded;
    int32_t stealth;
    int32_t aggression;
    int32_t time;
    int32_t saves_used;
};

using TGame = Struct<
    Game,
    Fields<
        Seek<0x2625d4>,
        Field<RawAddr<uint32_t>, &Game::engine_ptr>,
        Seek<0x2a6c50>,
        Field<Ref<TEntityManager>, &Game::entity_manager>,
        Seek<0x2a6c5c>,
        Field<Ref<TEngine>, &Game::engine>,
        Seek<0x28aa18>,
        Field<ZString<0x40>, &Game::lethed>,
        Seek<0x2b3418>,
        Field<ZString<0x8>, &Game::current_level_name>,
        Field<Int32, &Game::shots_fired>,
        Field<Int32, &Game::close_encounters>,
        Field<Int32, &Game::headshots>,
        Field<Int32, &Game::alerts>,
        Field<Int32, &Game::enemies_killed>,
        Field<Int32, &Game::enemies_wounded>,
        Field<Int32, &Game::innocents_killed>,
        Field<Int32, &Game::innocents_wounded>,
        Field<Int32, &Game::stealth>,
        Field<Int32, &Game::aggression>,
        Field<Int32, &Game::time>,
        Field<Int32, &Game::saves_used>>>;

}  // namespace hitman2_silent_assassin::structs
