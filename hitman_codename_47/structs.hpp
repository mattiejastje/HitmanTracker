#pragma once

#include <cstdint>
#include <mempeep/descriptors.hpp>
#include <optional>
#include <string>
#include <vector>

using namespace mempeep;

namespace hitman_codename_47::structs {

struct EntityManagerData {
    RemoteValue<Primitive<std::array<int32_t, 0x40000>>, uint32_t> versions;
    RemoteValue<Primitive<std::array<uint32_t, 0x40000>>, uint32_t> entities;
};

using TEntityManagerData = Struct<
    EntityManagerData,
    Fields<
        Seek<0x9>,
        Field<
            Ref<RemoteAddr<Primitive<std::array<int32_t, 0x40000>>, uint32_t>>,
            &EntityManagerData::versions>,
        Field<
            Ref<RemoteAddr<Primitive<std::array<uint32_t, 0x40000>>, uint32_t>>,
            &EntityManagerData::entities>>>;

struct EntityManager {
    EntityManagerData data;
};

using TEntityManager = Struct<
    EntityManager,
    Fields<Field<Ref<TEntityManagerData>, &EntityManager::data>>>;

struct SmallString {
    std::string text;
};

using TSmallString = Struct<
    SmallString,
    Fields<Field<Ref<ZString<0x100>>, &SmallString::text>, Skip<0x7c>>>;

struct SceneNode {
    int8_t unk_flag_1;
    int8_t unk_flag_2;
    std::optional<SmallString> scene_name;
    double scene_creation_time;
    uint32_t prev_node;
    uint32_t next_node;
};

using TSceneNode = Struct<
    SceneNode,
    Fields<
        Seek<-0x10c>,
        Skip<0x4>,
        Field<Int8, &SceneNode::unk_flag_1>,
        Field<Int8, &SceneNode::unk_flag_2>,
        Field<NullableRef<TSmallString>, &SceneNode::scene_name>,
        Seek<-0x32>,
        Field<Double, &SceneNode::scene_creation_time>,
        Seek<0x0>,
        Field<RawAddr<uint32_t>, &SceneNode::prev_node>,
        Field<RawAddr<uint32_t>, &SceneNode::next_node>>>;

struct SceneContainer {
    uint32_t vtable;
    uint32_t flags;
    int32_t link_offset;
    std::vector<SceneNode> scenes;
    uint32_t scenes_tail;
};

using TSceneContainer = Struct<
    SceneContainer,
    Fields<
        Field<RawAddr<uint32_t>, &SceneContainer::vtable>,
        Skip<0x8>,
        Field<UInt32, &SceneContainer::flags>,
        Skip<0x8>,
        Field<Int32, &SceneContainer::link_offset>,
        Field<
            List<
                TSceneNode,
                &SceneNode::next_node,
                ListKind::NULL_TERMINATED,
                0x100>,
            &SceneContainer::scenes>,
        Field<RawAddr<uint32_t>, &SceneContainer::scenes_tail>>>;

struct SceneManager {
    int8_t unk_flag_7d;
    std::optional<SceneContainer> scene_container;
};

using TSceneManager = Struct<
    SceneManager,
    Fields<
        Seek<0x7d>,
        Field<Int8, &SceneManager::unk_flag_7d>,
        Field<NullableRef<TSceneContainer>, &SceneManager::scene_container>>>;

struct UnkNode {
    uint32_t vtable;
    int32_t unk2;
    uint32_t next_node;
};

struct PropertyManager {
    uint32_t unk_data;
};

using TPropertyManager = Struct<
    PropertyManager,
    Fields<Seek<0xd>, Field<RawAddr<uint32_t>, &PropertyManager::unk_data>>>;

struct EngineData {
    SceneManager scene_manager;
    std::string mounted_archive;
    double cpu_cycles_per_second;
    float unk_a71;
    float unk_a75;
    double game_time;
    double real_time;
    double game_time_copy;
    double game_time_previous;
    RemoteValue<
        List<
            Primitive<UnkNode>,
            &UnkNode::next_node,
            ListKind::NULL_TERMINATED,
            0x1000>,
        uint32_t>
        unk_nodes;
    int8_t unk_flag_38f0;
    int8_t timestep_mode;
    PropertyManager property_manager;
    float game_speed_scale;
    int8_t unk_flag_3bca;
    int64_t cpu_cycles;
    double frame_time;
    uint32_t unk_3bdb;
};

using TEngineData = Struct<
    EngineData,
    Fields<
        Seek<0x59>,
        Field<Ref<TSceneManager>, &EngineData::scene_manager>,
        Seek<0x2e1>,
        Field<Ref<ZString<0x40>>, &EngineData::mounted_archive>,
        Seek<0xa61>,
        Field<Double, &EngineData::cpu_cycles_per_second>,
        Skip<0x8>,
        Field<Float, &EngineData::unk_a71>,
        Field<Float, &EngineData::unk_a75>,
        Seek<0x37b5>,
        Field<Double, &EngineData::game_time>,
        Field<Double, &EngineData::real_time>,
        Field<Double, &EngineData::game_time_copy>,
        Field<Double, &EngineData::game_time_previous>,
        Skip<0x10>,
        Field<
            RemoteAddr<
                List<
                    Primitive<UnkNode>,
                    &UnkNode::next_node,
                    ListKind::NULL_TERMINATED,
                    0x1000>,
                uint32_t>,
            &EngineData::unk_nodes>,
        Seek<0x38f0>,
        Field<Int8, &EngineData::unk_flag_38f0>,
        Field<Int8, &EngineData::timestep_mode>,
        Seek<0x3905>,
        Field<Ref<TPropertyManager>, &EngineData::property_manager>,
        Seek<0x398e>,
        Field<Float, &EngineData::game_speed_scale>,
        Seek<0x3bca>,
        Field<Int8, &EngineData::unk_flag_3bca>,
        Field<Int64, &EngineData::cpu_cycles>,
        Field<Double, &EngineData::frame_time>,
        Field<RawAddr<uint32_t>, &EngineData::unk_3bdb>>>;

struct Engine {
    EngineData engine_data;
};

using TEngine
    = Struct<Engine, Fields<Field<Ref<TEngineData>, &Engine::engine_data>>>;

struct HitmanDlc {
    EntityManager entity_manager;
    Engine engine;
    uint32_t unk_1f0010;
    float unk_zero_1f03c4;
    float unk_float_245e1c;
    int32_t unk_counter_1;
    double unk_game_time_1;
    int32_t unk_counter_2;
    double unk_game_time_2;
};

using THitmanDlc = Struct<
    HitmanDlc,
    Fields<
        Seek<0x1f0008>,
        Field<Ref<TEntityManager>, &HitmanDlc::entity_manager>,
        Field<Ref<TEngine>, &HitmanDlc::engine>,
        Field<RawAddr<uint32_t>, &HitmanDlc::unk_1f0010>,
        Seek<0x1f03c4>,
        Field<Float, &HitmanDlc::unk_zero_1f03c4>,
        Seek<0x245e1c>,
        Field<Float, &HitmanDlc::unk_float_245e1c>,
        Skip<0x8>,
        Field<Int32, &HitmanDlc::unk_counter_1>,
        Skip<0x4>,
        Field<Double, &HitmanDlc::unk_game_time_1>,
        Field<Int32, &HitmanDlc::unk_counter_2>,
        Skip<0x4>,
        Field<Double, &HitmanDlc::unk_game_time_2>>>;

}  // namespace hitman_codename_47::structs
