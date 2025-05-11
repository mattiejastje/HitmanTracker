#include "stats.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

#include "../logging.hpp"
#include "../mem/read_write.hpp"

// the scenes\*.gms appear when restarting
std::unordered_map<std::string, int> scenes = {
    {"hitmanbloodmoney", 0},
    {"scenes/hitmanbloodmoney.gms", 0},
    {"saveandcontinue", 0},
    // Hideout
    {"hideout/hideout_main", 1},
    // Death of a Showman
    {"m00/m00_intro", 2},
    {"m00/m00_main", 2},
    {"scenes/m00/m00_main.gms", 2},
    {"m00/m00_news", 2},
    {"m00/m00_albino", 2},
    // A Vintage Year
    {"m01/m01_premission", 3},
    {"m01/m01_main", 3},
    {"scenes/m01/m01_main.gms", 3},
    {"m01/m01_postmission", 3},
    {"m01/m01_news", 3},
    // Curtains Down
    {"m03/m03_premission", 4},
    {"m03/m03_main", 4},
    {"scenes/m03/m03_main.gms", 4},
    {"m03/m03_postmission", 4},
    {"m03/m03_news", 4},
    // Flatline
    {"m04/m04_premission", 5},
    {"m04/m04_main", 5},
    {"scenes/m04/m04_main.gms", 5},
    {"m04/m04_postmission", 5},
    {"m04/m04_news", 5},
    // A New Life
    {"m05/m05_premission", 6},
    {"m05/m05_main", 6},
    {"scenes/m05/m05_main.gms", 6},
    {"m05/m05_postmission", 6},
    {"m05/m05_news", 6},
    // Murder of the Crows
    {"m06/m06_premission", 7},
    {"m06/m06_main", 7},
    {"scenes/m06/m06_main.gms", 7},
    {"m06/m06_postmission", 7},
    {"m06/m06_news", 7},
    // You Better Watch Out
    {"m02/m02_premission", 8},
    {"m02/m02_main", 8},
    {"scenes/m02/m02_main.gms", 8},
    {"m02/m02_postmission", 8},
    {"m02/m02_news", 8},
    // Death on the Mississippi
    {"m08/m08_premission", 9},
    {"m08/m08_main", 9},
    {"scenes/m08/m08_main.gms", 9},
    {"m08/m08_postmission", 9},
    {"m08/m08_news", 9},
    // Till Death Do Us Part
    {"m09/m09_premission", 10},
    {"m09/m09_main", 10},
    {"scenes/m09/m09_main.gms", 10},
    {"m09/m09_postmission", 10},
    {"m09/m09_news", 10},
    // A House of Cards
    {"m10/m10_premission", 11},
    {"m10/m10_main", 11},
    {"scenes/m10/m10_main.gms", 11},
    {"m10/m10_postmission", 11},
    {"m10/m10_news", 11},
    // A Dance with The Devil
    {"m11/m11_premission", 12},
    {"m11/m11_main", 12},
    {"scenes/m11/m11_main.gms", 12},
    {"m11/m11_postmission", 12},
    {"m11/m11_news", 12},
    // Amendment XXV
    {"m12/m12_premission", 13},
    {"m12/m12_main", 13},
    {"scenes/m12/m12_main.gms", 13},
    {"m12/m12_postmission", 13},
    {"m12/m12_news", 13},
    // Requiem
    {"m13/m13_intro", 14},
    {"m13/m13_main", 14},
    {"scenes/m13/m13_main.gms", 14},
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

void hitman_blood_money::update_slow(
    void* handle,
    const BasePtrs& base_ptrs,
    int32_t hook_target_ptr,
    Stats& stats
) {
    auto scene = read_string(handle, hook_target_ptr, 64);
    if (!scene) return;
    logging::trace("Scene {}", scene.value());
    auto iter = scenes.find(scene.value());
    if (iter != scenes.end()) {
        stats.map = iter->second;
        logging::trace("map {}", stats.map);
    } else {
        if (!scene.value().empty()) {
            logging::error("No map registered for scene {}", scene.value());
        }
        stats.map = 0;
    }
}

void hitman_blood_money::update_fast(
    void* handle,
    const BasePtrs& base_ptrs,
    int32_t hook_target_ptr,
    Stats& stats
) {
    if (stats.map > 0) {
        stats.time = read<int32_t>(handle, base_ptrs[0] + 0x41F820, {0x48})
                         .value_or(stats.time)
                     * 0.0009765625f;  // 1 / 1024.0f
    }
}
