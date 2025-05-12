#include "stats.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

#include "../logging.hpp"
#include "../mem/read_write.hpp"

/*

Notes.

- The scenes\*.gms appear when restarting; they load the same scene as their
shorter equivalents.

- During premission, main, and postmission, the timer restarts each time.

- During main, the GameStats do not update time, witnesses, suit_left_on_level,
and custom_weapons_left_on_level. They may contain outdated values.

- During postmission, the timer restarts. However, the GameStats time will then
have the correct mission time and all the other correct values.

*/

struct Scene {
    int map{0};
    MapStage map_stage{MapStage::pre};
};

std::unordered_map<std::string, Scene> scenes = {
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
struct GameStats {
    int32_t unknown00;                     // 0x0000
    int32_t rating1;                       // 0x0004
    int32_t rating0;                       // 0x0008
    int32_t special_rating;                // 0x000C
    int32_t rating1_total;                 // 0x0010
    int32_t rating0_total;                 // 0x0014
    int32_t current_level;                 // 0x0018
    int32_t shots_fired;                   // 0x001C
    int32_t shots_hit;                     // 0x0020
    int32_t shots_missed;                  // 0x0024
    int32_t headshots;                     // 0x0028
    int32_t clean_kills;                   // 0x002C
    int32_t target_clean_kills;            // 0x0030
    int32_t accident_kills;                // 0x0034
    int32_t alarms;                        // 0x0038
    int32_t witnesses;                     // 0x003C
    int32_t enemies_killed;                // 0x0040
    int32_t unknown01;                     // 0x0044
    int32_t enemies_wounded;               // 0x0048
    int32_t enemies_pushed_to_death;       // 0x004C
    int32_t police_killed;                 // 0x0050
    int32_t police_wounded;                // 0x0054
    int32_t innocents_killed;              // 0x0058
    int32_t innocents_wounded;             // 0x005C
    int32_t unknown02;                     // 0x0060
    int32_t unknown03;                     // 0x0064
    int32_t targets_kniped;                // 0x0068
    int32_t targets_killed;                // 0x006C
    int32_t unknown04;                     // 0x0070
    int32_t fiberwire_kills;               // 0x0074
    int32_t close_combat_kills;            // 0x0078
    int32_t preferred_weapon;              // 0x007C
    int32_t unknown05;                     // 0x0080
    int32_t noisy_shots;                   // 0x0084
    int32_t num_saves;                     // 0x0088
    int32_t bodies_hidden;                 // 0x008C
    int32_t disguises_used;                // 0x0090
    int32_t impersonations;                // 0x0094
    int32_t agency_pickups;                // 0x0098
    int32_t time;                          // 0x009C
    int32_t money;                         // 0x00A0
    int32_t suit_left_on_level;            // 0x00A4
    int32_t target1_killed_with;           // 0x00A8
    int32_t target2_killed_with;           // 0x00AC
    int32_t target3_killed_with;           // 0x00B0
    int32_t target4_killed_with;           // 0x00B4
    int32_t target5_killed_with;           // 0x00B8
    int32_t target6_killed_with;           // 0x00BC
    int32_t main_target_number;            // 0x00C0
    int32_t targets_poisoned;              // 0x00C4
    int32_t frisk_failed;                  // 0x00C8
    int32_t ghost_failed;                  // 0x00CC
    int32_t bodies_found;                  // 0x00D0
    int32_t target_bodies_found;           // 0x00D4
    int32_t unconscious_bodies_found;      // 0x00D8
    int32_t cover_blown;                   // 0x00DC
    int32_t unknown07;                     // 0x00E0
    int32_t notoriety;                     // 0x00E4
    int32_t total_notoriety;               // 0x00E8
    int32_t camera_caught;                 // 0x00EC
    int32_t unknown08;                     // 0x00F0
    int32_t custom_weapons_left_on_level;  // 0x00F4
    int32_t custom_sniper_silenced;        // 0x00F8
    int32_t custom_hardballer_silenced;    // 0x00FC
    int32_t custom_sg_silenced;            // 0x0100
    int32_t custom_mg_silenced;            // 0x0104
    int32_t custom_smg_silenced;           // 0x0108
};

struct SuitPtrs {
    int32_t current_suit;
    int32_t starting_suit;
};

static Status status(int32_t value, bool required = true) {
    return required ? (value ? Status::RED : Status::YELLOW) : Status::GREEN;
}

static StatsValue stats_value(int32_t value, bool required = true) {
    return StatsValue{value, status(value, required)};
}

void hitman_blood_money::update_slow(
    void* handle,
    const BasePtrs& base_ptrs,
    int32_t hook_target_ptr,
    Stats& stats
) {
    stats.difficulty = read<int32_t>(handle, base_ptrs[0] + 0x41F83C, {0x6664})
                           .value_or(stats.difficulty);
    auto scene = read_string(handle, hook_target_ptr, 64);
    if (!scene) return;
    logging::trace("Scene {}", scene.value());
    auto iter = scenes.find(scene.value());
    if (iter != scenes.end()) {
        stats.map = iter->second.map;
        stats.map_stage = iter->second.map_stage;
        logging::trace(
            "Map {}, stage {}",
            stats.map,
            stats.map_stage == MapStage::pre    ? "pre"
            : stats.map_stage == MapStage::main ? "main"
                                                : "post"
        );
    } else {
        if (!scene.value().empty()) {
            logging::error("No map registered for scene {}", scene.value());
        }
    }
    if (stats.map > 0) {
        GameStats game_stats = {0};
        // note that pointers/values are not ready until timer starts running
        if (stats.map_stage != MapStage::pre && stats.time > 0.1f) {
            if (!read_bytes(
                    handle,
                    base_ptrs[0] + 0x5B2538,
                    &game_stats,
                    sizeof(game_stats)
                )) {
                logging::error("Unable to read game stats");
            }
        }
        // fix outdated values in main stage
        if (stats.map_stage == MapStage::main && stats.time > 0.1f) {
            game_stats.suit_left_on_level = 0;  // overwrite outdated value
            auto suit_ptrs = read<SuitPtrs>(
                handle, base_ptrs[0] + 0x41F83C, {0x0A40, 0x0FD0}
            );
            if (suit_ptrs) {
                game_stats.suit_left_on_level
                    = (suit_ptrs.value().current_suit
                       != suit_ptrs.value().starting_suit);
                logging::trace("Suit left {}", stats.suit_left.value);
            } else {
                logging::warn("Unable to read suit pointers");
            }
            // TODO get next two through hook
            game_stats.custom_weapons_left_on_level = 0;
            game_stats.witnesses = 0;
        }
        // set up the stats
        stats.innocents_killed = stats_value(game_stats.innocents_killed);
        stats.innocents_wounded = stats_value(game_stats.innocents_wounded);
        stats.enemies_killed = stats_value(game_stats.enemies_killed);
        stats.enemies_wounded = stats_value(game_stats.enemies_wounded);
        stats.police_killed = stats_value(game_stats.police_killed);
        stats.police_wounded = stats_value(game_stats.police_wounded);
        stats.frisk_failed = stats_value(game_stats.frisk_failed);
        stats.cover_blown = stats_value(game_stats.cover_blown);
        stats.bodies_fnd = stats_value(game_stats.bodies_found);
        stats.target_bodies_fnd
            = stats_value(game_stats.target_bodies_found, stats.difficulty > 1);
        stats.uncon_bodies_fnd
            = stats_value(game_stats.unconscious_bodies_found);
        stats.on_camera
            = stats_value(game_stats.camera_caught ? 1 : 0);  // 2 -> 1
        stats.cust_weapons_left.status = status(
            game_stats.custom_weapons_left_on_level, stats.difficulty > 2
        );
        stats.suit_left
            = stats_value(game_stats.suit_left_on_level, stats.difficulty > 2);
        stats.witnesses = stats_value(game_stats.witnesses);
        // silent assassin
        bool items_left_on_map = stats.difficulty > 2
                                 && (stats.cust_weapons_left.value != 0
                                     || stats.suit_left.value != 0);
        stats.silent_assassin
            = (stats.innocents_killed.value != 0
               || stats.innocents_wounded.value != 0
               || stats.enemies_killed.value != 0
               || stats.enemies_wounded.value != 0
               || stats.police_killed.value != 0
               || stats.police_wounded.value != 0
               || stats.frisk_failed.value != 0 || stats.cover_blown.value != 0
               || stats.bodies_fnd.value != 0
               || (stats.difficulty > 1 && stats.target_bodies_fnd.value != 0)
               || stats.uncon_bodies_fnd.value != 0)
                  ? Status::RED
              : items_left_on_map || (stats.witnesses.value != 0)
                      || (stats.on_camera.value != 0)
                  ? Status::YELLOW
                  : Status::GREEN;
    }
}

static std::optional<int32_t> get_time(
    void* handle, const BasePtrs& base_ptrs, MapStage map_stage
) {
    if (map_stage == MapStage::main)
        return read<int32_t>(handle, base_ptrs[0] + 0x41F820, {0x48});
    if (map_stage == MapStage::post)
        return read<int32_t>(handle, base_ptrs[0] + 0x5B2538 + 0x009C);
    // map_stage == MapStage::pre
    return 0;
}

void hitman_blood_money::update_fast(
    void* handle,
    const BasePtrs& base_ptrs,
    int32_t hook_target_ptr,
    Stats& stats
) {
    if (stats.map > 0) {
        auto time = get_time(handle, base_ptrs, stats.map_stage);
        if (time) {
            stats.time = time.value() * 0.0009765625f; // 1 / 1024.0f
        } else {
            logging::error("Unable to read time");
        }
    }
}