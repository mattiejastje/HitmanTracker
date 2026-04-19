#pragma once

#include <array>
#include <cstdint>
#include <mempeep/descriptors.hpp>

using namespace mempeep;

namespace hitman_blood_money::structs {

struct TimeManager {
  int32_t time;
};

using TTimeManager = Struct<
  TimeManager,
  Fields<
    Seek<0x48>,
    Field<Int32, &TimeManager::time>>>;

struct Suits {
  int32_t current_suit;
  int32_t starting_suit;
};

struct SuitContainer {
  Suits suits;
};

using TSuitContainer = Struct<
  SuitContainer,
  Fields<
    Seek<0xfd0>,
    Field<Primitive<Suits>, &SuitContainer::suits>>>;

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
  TimeManager time_manager;
  std::optional<Settings> settings;
  std::array<int32_t, 0x42> stats;
};

using TGame = Struct<
  Game,
  Fields<
    Seek<0x41f820>,
    Field<Ref<TTimeManager>, &Game::time_manager>,
    Seek<0x41f83c>,
    Field<NullableRef<TSettings>, &Game::settings>,
    Seek<0x5b2538>,
    Field<Primitive<std::array<int32_t, 0x42>>, &Game::stats>>>;

}
