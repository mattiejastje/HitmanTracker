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

struct SceneManagerStats {
    SceneInfo info;
};

using TSceneManagerStats = Struct<
    SceneManagerStats,
    Fields<Seek<0x24>, Field<Ref<TSceneInfo>, &SceneManagerStats::info>>>;

struct SysInterfaceStats {
    std::optional<SceneManagerStats> scene_manager;
};

using TSysInterfaceStats = Struct<
    SysInterfaceStats,
    Fields<
        Seek<0xb8>,
        Field<
            NullableRef<TSceneManagerStats>,
            &SysInterfaceStats::scene_manager>>>;

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

struct GameStats {
    std::optional<SysInterfaceStats> sys_interface;
    std::optional<Settings> settings;
    std::array<int32_t, 0x42> stats;
};

using TGameSteamStats = Struct<
    GameStats,
    Fields<
        Seek<0x41f820>,
        Field<NullableRef<TSysInterfaceStats>, &GameStats::sys_interface>,
        Seek<0x41f83c>,
        Field<NullableRef<TSettings>, &GameStats::settings>,
        Seek<0x5b2538>,
        Field<Primitive<std::array<int32_t, 0x42>>, &GameStats::stats>>>;

struct SysInterfaceTimer {
    int32_t game_ticks;
};

using TSysInterfaceTimer = Struct<
    SysInterfaceTimer,
    Fields<Seek<0x48>, Field<Int32, &SysInterfaceTimer::game_ticks>>>;

struct GameTimer {
    std::optional<SysInterfaceTimer> sys_interface;
    int32_t stats_time;
};

using TGameSteamTimer = Struct<
    GameTimer,
    Fields<
        Seek<0x41f820>,
        Field<NullableRef<TSysInterfaceTimer>, &GameTimer::sys_interface>,
        Seek<0x5b25d4>,
        Field<Int32, &GameTimer::stats_time>>>;

using TGameGOGStats = Struct<
    GameStats,
    Fields<
        Seek<0x420820>,
        Field<NullableRef<TSysInterfaceStats>, &GameStats::sys_interface>,
        Seek<0x42083c>,
        Field<NullableRef<TSettings>, &GameStats::settings>,
        Seek<0x5b3b38>,
        Field<Primitive<std::array<int32_t, 0x42>>, &GameStats::stats>>>;

using TGameGOGTimer = Struct<
    GameTimer,
    Fields<
        Seek<0x420820>,
        Field<NullableRef<TSysInterfaceTimer>, &GameTimer::sys_interface>,
        Seek<0x5b3bd4>,
        Field<Int32, &GameTimer::stats_time>>>;

}  // namespace hitman_blood_money::structs
