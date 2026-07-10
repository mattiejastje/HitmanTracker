#pragma once

#include <array>
#include <cstdint>
#include <mempeep/descriptors.hpp>
#include <optional>
#include <string>

using namespace mempeep;

namespace hitman_blood_money::structs {

struct SceneInfo {
    int8_t unk_flag_00;
    std::string scene_name;
};

using TSceneInfo = Struct<
    SceneInfo,
    Fields<
        Field<Int8, &SceneInfo::unk_flag_00>,
        Skip<0x1>,
        Field<ZString<0x104>, &SceneInfo::scene_name>>>;

struct SceneTask {
    uint32_t prev_task;
    uint32_t next_task;
    float unk_tick_interval;
    int32_t unk_last_tick;
    int32_t unk_flags_1;
    int32_t unk_flags_2;
    uint32_t unk_ptr_24;
};

using TSceneTask = Struct<
    SceneTask,
    Fields<
        Field<RawAddr<uint32_t>, &SceneTask::prev_task>,
        Field<RawAddr<uint32_t>, &SceneTask::next_task>,
        Seek<0xc>,
        Field<Float, &SceneTask::unk_tick_interval>,
        Field<Int32, &SceneTask::unk_last_tick>,
        Skip<0x4>,
        Field<Int32, &SceneTask::unk_flags_1>,
        Field<Int32, &SceneTask::unk_flags_2>,
        Field<RawAddr<uint32_t>, &SceneTask::unk_ptr_24>>>;

struct SceneTaskScheduler {
    RemoteValue<
        Array<
            List<TSceneTask, &SceneTask::next_task, ListKind::CIRCULAR, 0x100>,
            0x9>,
        uint32_t>
        tasks;
    uint32_t unk_current_54;
    uint32_t unk_active_58;
};

using TSceneTaskScheduler = Struct<
    SceneTaskScheduler,
    Fields<
        Skip<0x4>,
        Field<
            RemoteAddr<
                Array<
                    List<
                        TSceneTask,
                        &SceneTask::next_task,
                        ListKind::CIRCULAR,
                        0x100>,
                    0x9>,
                uint32_t>,
            &SceneTaskScheduler::tasks>,
        Seek<0x54>,
        Field<RawAddr<uint32_t>, &SceneTaskScheduler::unk_current_54>,
        Field<RawAddr<uint32_t>, &SceneTaskScheduler::unk_active_58>>>;

struct SceneManager {
    SceneInfo info;
    SceneTaskScheduler task_scheduler;
    int8_t is_paused;
    uint32_t unk_52d0;
};

using TSceneManager = Struct<
    SceneManager,
    Fields<
        Seek<0x24>,
        Field<Ref<TSceneInfo>, &SceneManager::info>,
        Seek<0x30>,
        Field<TSceneTaskScheduler, &SceneManager::task_scheduler>,
        Seek<0xbc>,
        Field<Int8, &SceneManager::is_paused>,
        Seek<0x52d0>,
        Field<RawAddr<uint32_t>, &SceneManager::unk_52d0>>>;

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
    int32_t game_ticks_copy;
    int32_t game_ticks;
    int32_t game_ticks_previous;
    float game_frame_time;
    int32_t pause_ticks_offset;
    float qpc_frequency;
    std::optional<SceneManager> scene_manager;
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
        Skip<0x4>,
        Field<Int32, &SysInterface::qpc_ticks>,
        Field<Float, &SysInterface::qpc_delta>,
        Field<Int32, &SysInterface::game_ticks_copy>,
        Skip<0x4>,
        Field<Int32, &SysInterface::game_ticks>,
        Field<Int32, &SysInterface::game_ticks_previous>,
        Field<Float, &SysInterface::game_frame_time>,
        Field<Int32, &SysInterface::pause_ticks_offset>,
        Skip<0x8>,
        Field<Float, &SysInterface::qpc_frequency>,
        Seek<0xb8>,
        Field<NullableRef<TSceneManager>, &SysInterface::scene_manager>,
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
    std::optional<SysInterface> sys_interface;
    std::optional<Settings> settings;
    std::array<int32_t, 0x42> stats;
};

using TGameSteam = Struct<
    Game,
    Fields<
        Seek<0x41f820>,
        Field<NullableRef<TSysInterface>, &Game::sys_interface>,
        Seek<0x41f83c>,
        Field<NullableRef<TSettings>, &Game::settings>,
        Seek<0x5b2538>,
        Field<Primitive<std::array<int32_t, 0x42>>, &Game::stats>>>;

using TGameGOG = Struct<
    Game,
    Fields<
        Seek<0x420820>,
        Field<NullableRef<TSysInterface>, &Game::sys_interface>,
        Seek<0x42083c>,
        Field<NullableRef<TSettings>, &Game::settings>,
        Seek<0x5b3b38>,
        Field<Primitive<std::array<int32_t, 0x42>>, &Game::stats>>>;

}  // namespace hitman_blood_money::structs
