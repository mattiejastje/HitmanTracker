#pragma once

#include <array>
#include <cstdint>
#include <mempeep/descriptors.hpp>
#include <optional>

using namespace mempeep;

namespace hitman_blood_money::structs {

struct SceneManager {
    int8_t is_paused;
};

using TSceneManager = Struct<
    SceneManager,
    Fields<Seek<0xbc>, Field<Int8, &SceneManager::is_paused>>>;

struct SysInterface {
    uint32_t vtable;
    double clock_elapsed;
    double clock_current;
    double qpc_elapsed;
    int32_t clock_ticks;
    float clock_delta;
    int32_t clock_ticks_previous;
    int32_t qpc_ticks;
    float qpc_delta;
    int32_t game_ticks;
    int32_t game_ticks_previous;
    float game_frame_time;
    int32_t pause_ticks_offset;
    float qpc_frequency;
    SceneManager scene_manager;
    float requested_timescale;
    float timescale;
    int8_t is_timescale_locked;
    int8_t use_qpc;
    int64_t qpc_time_offset;
    int64_t qpc_last_sample;
    double qpc_frame_time;
    int8_t qpc_force_tick;
};

using TSysInterface = Struct<
    SysInterface,
    Fields<
        Field<RawAddr<uint32_t>, &SysInterface::vtable>,
        Skip<0xc>,
        Field<Double, &SysInterface::clock_elapsed>,
        Field<Double, &SysInterface::clock_current>,
        Field<Double, &SysInterface::qpc_elapsed>,
        Field<Int32, &SysInterface::clock_ticks>,
        Field<Float, &SysInterface::clock_delta>,
        Field<Int32, &SysInterface::clock_ticks_previous>,
        Seek<0x38>,
        Field<Int32, &SysInterface::qpc_ticks>,
        Field<Float, &SysInterface::qpc_delta>,
        Seek<0x48>,
        Field<Int32, &SysInterface::game_ticks>,
        Field<Int32, &SysInterface::game_ticks_previous>,
        Field<Float, &SysInterface::game_frame_time>,
        Field<Int32, &SysInterface::pause_ticks_offset>,
        Seek<0x60>,
        Field<Float, &SysInterface::qpc_frequency>,
        Seek<0xb8>,
        Field<Ref<TSceneManager>, &SysInterface::scene_manager>,
        Seek<0xb24>,
        Field<Float, &SysInterface::requested_timescale>,
        Field<Float, &SysInterface::timescale>,
        Field<Int8, &SysInterface::is_timescale_locked>,
        Seek<0xde1>,
        Field<Int8, &SysInterface::use_qpc>,
        Seek<0x11f8>,
        Field<Int64, &SysInterface::qpc_time_offset>,
        Seek<0x1438>,
        Field<Int64, &SysInterface::qpc_last_sample>,
        Field<Double, &SysInterface::qpc_frame_time>,
        Seek<0x1660>,
        Field<Int8, &SysInterface::qpc_force_tick>>>;

struct Suits {
    int32_t current_suit;
    int32_t starting_suit;
};

struct SuitContainer {
    Suits suits;
};

using TSuitContainer = Struct<
    SuitContainer,
    Fields<Seek<0xfd0>, Field<Primitive<Suits>, &SuitContainer::suits>>>;

struct Settings {
    std::optional<SuitContainer> suit_container;
    int32_t difficulty;
};

using TSettings = Struct<
    Settings,
    Fields<
        Seek<0xa40>,
        Field<NullableRef<TSuitContainer>, &Settings::suit_container>,
        Seek<0x6664>,
        Field<Bounded<Int32, 0, 3>, &Settings::difficulty>>>;

struct Game {
    SysInterface SysInterface;
    std::optional<Settings> settings;
    std::array<int32_t, 0x42> stats;
    float seconds_per_tick;
    double seconds_per_millisecond;
};

using TGame = Struct<
    Game,
    Fields<
        Seek<0x41f820>,
        Field<Ref<TSysInterface>, &Game::SysInterface>,
        Seek<0x41f83c>,
        Field<NullableRef<TSettings>, &Game::settings>,
        Seek<0x5b2538>,
        Field<Primitive<std::array<int32_t, 0x42>>, &Game::stats>,
        Seek<0x356108>,
        Field<Float, &Game::seconds_per_tick>,
        Seek<0x35ecd0>,
        Field<Double, &Game::seconds_per_millisecond>>>;

}  // namespace hitman_blood_money::structs
