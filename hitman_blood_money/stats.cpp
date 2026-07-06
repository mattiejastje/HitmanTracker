#include "stats.hpp"

#include <spdlog/spdlog.h>

#include <cstdint>
#include <mempeep/read.hpp>
#include <mempeep/tracers/log_tracer.hpp>
#include <string>
#include <unordered_map>

#include "../hitman_common/simple_rating.hpp"
#include "../mem/read_write.hpp"
#include "structs.hpp"

/*

Notes.

- The scenes\*.gms appear when restarting; they load the same scene as their
shorter equivalents.

- During main, the GameStats do not update time, witnesses, suit_left_on_level,
and custom_weapons_left_on_level. They may contain outdated values. These are
only correct during postmission.

- During premission, main, and postmission, engine timer restarts each time.

Solutions.

- Use engine timer for main, game stats time for postmission.

- Hook LevelControl.FrameUpdate to update witnesses and custom weapons left
during main.

- Use suit pointers during main (they are not valid premission or postmission)
to track whether suit was left.

*/

struct Scene {
    int map{0};
    MapStage map_stage{MapStage::pre};
};

const std::unordered_map<std::string, Scene> scenes = {
    {"hitmanbloodmoney", {}},
    {"scenes/hitmanbloodmoney.gms", {}},
    {"saveandcontinue", {}},
    // Hideout
    {"hideout/hideout_main", {1}},  // no stats
    // Death of a Showman
    {"m00/m00_intro", {2}},
    {"m00/m00_main", {2, MapStage::main}},
    {"scenes/m00/m00_main.gms", {2, MapStage::main}},
    {"m00/m00_news", {2, MapStage::post}},
    {"m00/m00_albino", {2, MapStage::post}},
    // A Vintage Year
    {"m01/m01_premission", {3}},
    {"m01/m01_main", {3, MapStage::main}},
    {"scenes/m01/m01_main.gms", {3, MapStage::main}},
    {"m01/m01_postmission", {3, MapStage::post}},
    {"m01/m01_news", {3, MapStage::post}},
    // Curtains Down
    {"m03/m03_premission", {4}},
    {"m03/m03_main", {4, MapStage::main}},
    {"scenes/m03/m03_main.gms", {4, MapStage::main}},
    {"m03/m03_postmission", {4, MapStage::post}},
    {"m03/m03_news", {4, MapStage::post}},
    // Flatline
    {"m04/m04_premission", {5}},
    {"m04/m04_main", {5, MapStage::main}},
    {"scenes/m04/m04_main.gms", {5, MapStage::main}},
    {"m04/m04_postmission", {5, MapStage::post}},
    {"m04/m04_news", {5, MapStage::post}},
    // A New Life
    {"m05/m05_premission", {6}},
    {"m05/m05_main", {6, MapStage::main}},
    {"scenes/m05/m05_main.gms", {6, MapStage::main}},
    {"m05/m05_postmission", {6, MapStage::post}},
    {"m05/m05_news", {6, MapStage::post}},
    // Murder of the Crows
    {"m06/m06_premission", {7}},
    {"m06/m06_main", {7, MapStage::main}},
    {"scenes/m06/m06_main.gms", {7, MapStage::main}},
    {"m06/m06_postmission", {7, MapStage::post}},
    {"m06/m06_news", {7, MapStage::post}},
    // You Better Watch Out
    {"m02/m02_premission", {8}},
    {"m02/m02_main", {8, MapStage::main}},
    {"scenes/m02/m02_main.gms", {8, MapStage::main}},
    {"m02/m02_postmission", {8, MapStage::post}},
    {"m02/m02_news", {8, MapStage::post}},
    // Death on the Mississippi
    {"m08/m08_premission", {9}},
    {"m08/m08_main", {9, MapStage::main}},
    {"scenes/m08/m08_main.gms", {9, MapStage::main}},
    {"m08/m08_postmission", {9, MapStage::post}},
    {"m08/m08_news", {9, MapStage::post}},
    // Till Death Do Us Part
    {"m09/m09_premission", {10}},
    {"m09/m09_main", {10, MapStage::main}},
    {"scenes/m09/m09_main.gms", {10, MapStage::main}},
    {"m09/m09_postmission", {10, MapStage::post}},
    {"m09/m09_news", {10, MapStage::post}},
    // A House of Cards
    {"m10/m10_premission", {11}},
    {"m10/m10_main", {11, MapStage::main}},
    {"scenes/m10/m10_main.gms", {11, MapStage::main}},
    {"m10/m10_postmission", {11, MapStage::post}},
    {"m10/m10_news", {11, MapStage::post}},
    // A Dance with The Devil
    {"m11/m11_premission", {12}},
    {"m11/m11_main", {12, MapStage::main}},
    {"scenes/m11/m11_main.gms", {12, MapStage::main}},
    {"m11/m11_postmission", {12, MapStage::post}},
    {"m11/m11_news", {12, MapStage::post}},
    // Amendment XXV
    {"m12/m12_premission", {13}},
    {"m12/m12_main", {13, MapStage::main}},
    {"scenes/m12/m12_main.gms", {13, MapStage::main}},
    {"m12/m12_postmission", {13, MapStage::post}},
    {"m12/m12_news", {13, MapStage::post}},
    // Requiem
    {"m13/m13_intro", {14}},
    {"m13/m13_main", {14, MapStage::main}},
    {"scenes/m13/m13_main.gms", {14, MapStage::main}},
};

// https://github.com/OrfeasZ/Statman/blob/master/StatModules/HM3/Src/HM3/Structs/HM3Stats.h
constexpr std::size_t SHOTS_HIT = 8;
constexpr std::size_t ACCIDENT_KILLS = 13;
constexpr std::size_t WITNESSES = 15;
constexpr std::size_t ENEMIES_KILLED = 16;
constexpr std::size_t ENEMIES_WOUNDED = 18;
constexpr std::size_t POLICE_KILLED = 20;
constexpr std::size_t POLICE_WOUNDED = 21;
constexpr std::size_t INNOCENTS_KILLED = 22;
constexpr std::size_t INNOCENTS_WOUNDED = 23;
constexpr std::size_t TIME = 39;
constexpr std::size_t SUIT_LEFT_ON_LEVEL = 41;
constexpr std::size_t FRISK_FAILED = 50;
constexpr std::size_t BODIES_FOUND = 52;
constexpr std::size_t TARGET_BODIES_FOUND = 53;
constexpr std::size_t UNCONSCIOUS_BODIES_FOUND = 54;
constexpr std::size_t COVER_BLOWN = 55;
constexpr std::size_t CAMERA_CAUGHT = 59;
constexpr std::size_t CUSTOM_WEAPONS_LEFT_ON_LEVEL = 61;

static std::optional<int32_t> get_time(
    void* handle, uintptr_t base_ptr, MapStage map_stage
) {
    if (map_stage == MapStage::main)
        // game.sys_interface.game_ticks
        return read<int32_t>(handle, base_ptr + 0x41F820, {0x48}, INT32_MAX);
    if (map_stage == MapStage::post)
        // game.stats[TIME]
        return read<int32_t>(handle, base_ptr + 0x5B2538 + 4 * TIME);
    // map_stage == MapStage::pre
    return 0;
}

static bool get_suit_left_on_level(
    MapStage map_stage, const hitman_blood_money::structs::Game& game
) {
    if (map_stage == MapStage::pre) {
        return 0;
    } else if (map_stage == MapStage::main) {
        if (game.settings && game.settings->suit_container) {
            auto& suits = game.settings->suit_container->suits;
            return suits.current_suit != suits.starting_suit;
        } else {
            return 0;  // not yet loaded
        }
    } else {
        // post-mission: stats are up to date
        return game.stats[SUIT_LEFT_ON_LEVEL];
    }
}

static Status get_rating_status(const hitman_blood_money::Stats& stats) {
    bool items_left_on_map
        = stats.difficulty > 2
          && (stats.cust_weapons_left.value != 0 || stats.suit_left.value != 0);
    return (stats.innocents_killed.value != 0
            || stats.innocents_wounded.value != 0
            || stats.enemies_killed.value != 0
            || stats.enemies_wounded.value != 0
            || stats.police_killed.value != 0 || stats.police_wounded.value != 0
            || stats.frisk_failed.value != 0 || stats.cover_blown.value != 0
            || stats.bodies_fnd.value != 0
            || (stats.difficulty > 1 && stats.target_bodies_fnd.value != 0)
            || stats.uncon_bodies_fnd.value != 0)
               ? Status::RED
           : items_left_on_map || (stats.witnesses.value != 0)
                   || (stats.on_camera.value != 0)
               ? Status::YELLOW
               : Status::GREEN;
};

GameStatsSlow hitman_blood_money::update_slow(Version version) {
    return [](const std::filesystem::path& exe_path,
              void* handle,
              const BasePtrs& base_ptrs,
              const LabelPtrs& label_ptrs,
              std::any& remote_state_any,
              std::any& stats_any) {
        auto& game = std::any_cast<structs::Game&>(remote_state_any);
        auto& stats = std::any_cast<Stats&>(stats_any);
        const RemoteValue<structs::TGame, uint32_t> remote_game{
            static_cast<uint32_t>(base_ptrs.at(0))
        };
        MemoryReader<uint32_t> reader{handle};
        auto tracer = mempeep::LogTracer{
            MempeepOnLogEntry{}, mempeep::LogLevel::ERRORS
        };
        if (!mempeep::read(remote_game, reader, tracer, game)) return false;
        auto& settings = game.settings;
        if (!settings) return true;  // game starting
        stats.difficulty = settings->difficulty;
        auto& sys_interface = game.sys_interface;
        if (!sys_interface) return true;  // game starting
        auto& scene_manager = sys_interface->scene_manager;
        if (!scene_manager) return true;  // game starting
        auto& scene = scene_manager->info.scene_name;
        spdlog::trace("Scene {}", scene);
        std::transform(scene.begin(), scene.end(), scene.begin(), [](char& c) {
            if (c == '\\') return '/';
            return static_cast<char>(std::tolower(c));
        });
        auto iter = scenes.find(scene);
        if (iter != scenes.end()) {
            stats.map = iter->second.map;
            stats.map_stage = iter->second.map_stage;
            spdlog::trace(
                "Map {}, stage {}",
                stats.map,
                stats.map_stage == MapStage::pre    ? "pre"
                : stats.map_stage == MapStage::main ? "main"
                                                    : "post"
            );
        } else {
            spdlog::error("No map registered for scene {}", scene);
        }
        if (stats.map > 0) {
            // force values to zero at mission start
            if (stats.time < 0.1f) game.stats = {0};
            stats.innocents_killed = stats_value(game.stats[INNOCENTS_KILLED]);
            stats.innocents_wounded
                = stats_value(game.stats[INNOCENTS_WOUNDED]);
            stats.enemies_killed = stats_value(game.stats[ENEMIES_KILLED]);
            stats.enemies_wounded = stats_value(game.stats[ENEMIES_WOUNDED]);
            stats.police_killed = stats_value(game.stats[POLICE_KILLED]);
            stats.police_wounded = stats_value(game.stats[POLICE_WOUNDED]);
            stats.frisk_failed = stats_value(game.stats[FRISK_FAILED]);
            stats.cover_blown = stats_value(game.stats[COVER_BLOWN]);
            stats.bodies_fnd = stats_value(game.stats[BODIES_FOUND]);
            stats.target_bodies_fnd = stats_value(
                game.stats[TARGET_BODIES_FOUND], stats.difficulty > 1
            );
            stats.uncon_bodies_fnd
                = stats_value(game.stats[UNCONSCIOUS_BODIES_FOUND]);
            stats.witnesses = stats_value(game.stats[WITNESSES]);
            stats.on_camera
                = stats_value(game.stats[CAMERA_CAUGHT] ? 1 : 0);  // 2 -> 1
            stats.cust_weapons_left = stats_value(
                game.stats[CUSTOM_WEAPONS_LEFT_ON_LEVEL], stats.difficulty > 2
            );
            stats.suit_left = stats_value(
                get_suit_left_on_level(stats.map_stage, game),
                stats.difficulty > 2
            );
            auto status = get_rating_status(stats);
            stats.rating = {get_simple_rating_value(status), status};
            // extra stats not affecting silent assassin
            stats.shots_hit = game.stats[SHOTS_HIT];
            stats.accident_kills = game.stats[ACCIDENT_KILLS];
        }
        return true;
    };
}

// ticks are 1 / 1024 but final game screen shows 1 / 1000
// this seems to be a bug in the game
// here we use the mission time as shown by the game
// timer will run too fast but will be consistent with final mission screen
constexpr float game_display_seconds_per_tick = 1.0f / 1000;

GameStatsFast hitman_blood_money::update_fast(Version version) {
    return [](void* handle,
              const BasePtrs& base_ptrs,
              const LabelPtrs& label_ptrs,
              std::any& stats_any) {
        auto& stats = std::any_cast<Stats&>(stats_any);
        if (stats.map > 0) {
            const auto& base_ptr = base_ptrs.at(0);
            auto time = get_time(handle, base_ptr, stats.map_stage);
            if (time) stats.time = time.value() * game_display_seconds_per_tick;
            return time.has_value();
        }
        return true;
    };
}