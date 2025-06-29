#include "stats.hpp"

#include <format>
#include <unordered_map>

#include "../hitman_common/stats.hpp"
#include "../logging.hpp"
#include "../mem/read_write.hpp"

// https://github.com/pavledev/HitmanAbsolutionSDK/blob/4599042d197d8ead470c66ea002e6a9e573ae8ae/HitmanAbsolutionSDK/include/Glacier/ZGameTimeManager.h
struct GameTimeManager {
    int32_t vtable;
    int32_t _pad0;
    int64_t ticks_per_second;
    int64_t last_time_ticks;
    int64_t game_time;
    int64_t game_time_prev;
    int64_t game_time_delta;
    int64_t real_time;
    int64_t real_time_prev;
    int64_t real_time_delta;
    float game_time_multiplier;
    float debug_time_multiplier;
    int64_t frame_wait;
    int64_t frame_step;
    int64_t frame_remain;
    int32_t is_paused;
    int32_t frame_count;
    int32_t is_forced_time_step;
    int32_t _pad1;
    double forced_time_step;
    double forced_time_target;
};

static_assert(sizeof(GameTimeManager) == 0x88);

// https://github.com/pavledev/HitmanAbsolutionSDK/blob/main/HitmanAbsolutionSDK/include/Glacier/ZLevelManager.h
// https://github.com/pavledev/HitmanAbsolutionSDK/blob/main/HitmanAbsolutionSDK/include/Glacier/SSceneParameters.h
struct LevelManager {
    int32_t vtable;
    int32_t scene_resource_length;
    int32_t scene_resource_ptr;
    int32_t game_mode;  // 1 = story mode
    int64_t bonus_weapon;
    int64_t bonus_outfit;
    int32_t checkpoint_index;  // checkpoint the game was loaded from, not
                               // necessarily current checkpoint
    int8_t is_restoring;
    int8_t use_savegame;
};

enum class Rating { unrated, veteran, specialist, shadow, silent_assassin };

struct MapInfo {
    int map;
    int num_evidence;
    Rating max_rating;
};

const std::unordered_map<std::string, std::vector<MapInfo>> scenes = {
    {"assembly:/Scenes/L01/L01_Main.entity",
     {
         {1, 0},                  // 0 garden
         {2, 0},                  // 1 greenhouse
         {3, 1},                  // 2 cliffside
         {4, 1, Rating::shadow},  // 3 mansion ground
         {5, 0},                  // 4 mansion 2nd
     }},
    {"assembly:/Scenes/L02/L02_Main.entity",
     {
         {6, 1, Rating::silent_assassin},  // 0 king of chinatown
     }},
    {"assembly:/Scenes/L03/L03_Main.entity",
     {
         {9, 0, Rating::specialist},  // 0 burning hotel
         {10, 1, Rating::shadow},     // 1 library
         {0},                         // non-existing checkpoint
         {11, 0},                     // 3 pigeon coop
         {12, 1, Rating::shadow},     // 4 shangri-la
         {13, 1, Rating::shadow},     // 5 train station
     }},
    {"assembly:/Scenes/L04/L04_Main.entity",
     {
         {7, 1, Rating::shadow},  // 0 terminus hotel
         {8, 1, Rating::shadow},  // 1 upper floors
     }},
    {"assembly:/Scenes/L05/L05_Main.entity",
     {
         {14, 1, Rating::shadow},           // 0 courtyard
         {15, 1, Rating::silent_assassin},  // 1 vixen club
         {16, 0, Rating::specialist},       // 2 dressing rooms
         {17, 1, Rating::shadow},           // 3 derelict building
         {18, 1, Rating::shadow},           // 4 convenience store
         {19, 1},                           // 5 loading area
         {20, 1, Rating::silent_assassin},  // 6 chinese new year
     }},
    {"assembly:/Scenes/L06/L06_Main.entity",
     {
         {21, 0},                           // 0 victoria's ward
         {22, 1, Rating::shadow},           // 1 orphanage halls
         {0},                               // 2 "sister mary" cutscene
         {23, 1, Rating::silent_assassin},  // 3 central heating
     }},
    {"assembly:/Scenes/L07/L07_Main.entity",
     {
         {24, 1, Rating::shadow},  // 0 great balls of fire
     }},
    {"assembly:/Scenes/L08/L08_Main.entity",
     {
         {25, 1, Rating::veteran},  // 0 gunshop
     }},
    {"assembly:/Scenes/L09/L09_Main.entity",
     {
         {26, 1, Rating::silent_assassin},  // 0 streets of hope
         {27, 1, Rating::silent_assassin},  // 1 barber shop
     }},
    {"assembly:/Scenes/L10/L10_Main.entity",
     {
         {28, 0},  // 0 the desert
     }},
    {"assembly:/Scenes/L11/L11_Main.entity",
     {
         {29, 1, Rating::shadow},  // 0 dead end
         {30, 1, Rating::shadow},  // 1 old mill
         {31, 1, Rating::shadow},  // 2 descent
         {32, 1, Rating::shadow},  // 3 factory compound
     }},
    {"assembly:/Scenes/L12/L12_Main.entity",
     {
         {33, 1, Rating::silent_assassin},  // 0 test facility
         {34, 1, Rating::shadow},           // 1 decontamination
         {35, 1, Rating::silent_assassin},  // 2 r&d
     }},
    {"assembly:/Scenes/L13/L13_Main.entity",
     {
         {36, 1, Rating::shadow},           // 0 patriot's hangar
         {37, 1, Rating::silent_assassin},  // 1 arena
     }},
    {"assembly:/Scenes/L14/L14_Main.entity",
     {
         {38, 1, Rating::silent_assassin},  // 0 parking
         {39, 1, Rating::silent_assassin},  // 1 reception
         {40, 1, Rating::silent_assassin},  // 2 cornfield
     }},
    {"assembly:/Scenes/L17/L17_Main.entity",
     {
         {41, 1, Rating::shadow},  // courthouse
         {42, 1},                  // holding cells
         {43, 1, Rating::shadow},  // prison
     }},
    {"assembly:/Scenes/L18/L18_Main.entity",
     {
         {44, 1, Rating::shadow},  // 0 county jail
         {45, 1, Rating::shadow},  // 1 outgunned
         {46, 1, Rating::shadow},  // 2 burn
         {47, 1, Rating::shadow},  // 3 hope fair
         {48, 0},                  // 4 church
     }},
    {"assembly:/Scenes/L21/L21_Main.entity",
     {
         {49, 1},  // 0 tailor shop
     }},
    {"assembly:/Scenes/L22/L22_Main.entity",
     {
         {0},                               // non-existing checkpoint
         {50, 1, Rating::shadow},           // 1 blackwater park
         {51, 1, Rating::silent_assassin},  // 2 the penthouse
     }},
    {"assembly:/Scenes/L24/L24_Main.entity",
     {
         {52, 1, Rating::silent_assassin},  // 0 blackwater roof
     }},
    {"assembly:/Scenes/L25/L25_Main.entity",
     {
         {53, 1, Rating::shadow},           // 0 cemetary entrance
         {54, 1, Rating::silent_assassin},  // 1 burnwood family tomb
         {55, 1, Rating::silent_assassin},  // 2 crematorium
     }},
};

static Status get_rating_status(Rating max_rating, const Stats& stats) {
    return (max_rating != Rating::unrated && stats.spotted.value != 0)
               ? Status::RED
           : (max_rating != Rating::unrated
              && max_rating != Rating::silent_assassin
              && stats.evidence_left.value != 0)
               ? Status::YELLOW
               : Status::GREEN;
};

static std::string get_rating_value(Rating max_rating, Status status) {
    if (max_rating == Rating::unrated) {
        return "Unrated";
    }
    return std::format(
        "{}{}",
        status == Status::GREEN ? "" : "No ",
        max_rating == Rating::veteran      ? "Veteran"
        : max_rating == Rating::specialist ? "Specialist"
        : max_rating == Rating::shadow     ? "Shadow"
                                           : "Silent Assassin"
    );
}

void hitman_absolution::update_slow(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    auto difficulty = read<int32_t>(handle, base_ptrs[0] + 0xD58D04);
    if (difficulty) {
        stats.difficulty = difficulty.value();
    } else {
        logging::error("Unable to read difficulty");
    }
    auto level_manager = read<LevelManager>(handle, base_ptrs[0] + 0xE21310);
    if (!level_manager) {
        logging::error("Unable to read level manager");
        return;
    }
    if (level_manager.value().scene_resource_length > 64) {
        logging::error("Scene resource length overflow");
        return;
    }
    if (level_manager.value().scene_resource_length < 0
        || level_manager.value().game_mode != 1) {
        stats.map = 0;
        return;
    }
    auto scene = read_string(
        handle,
        level_manager.value().scene_resource_ptr,
        level_manager.value().scene_resource_length
    );
    if (!scene) {
        logging::error("Unable to read scene");
        return;
    }
    auto checkpoint = read<int32_t>(handle, base_ptrs[0] + 0xD60F94).value_or(-1);
    logging::trace("Scene {}, checkpoint {}", scene.value(), checkpoint);
    auto iter = scenes.find(scene.value());
    if (iter == scenes.end()) {
        logging::error("No map registered for scene {}", scene.value());
        return;
    } else if (checkpoint < 0 || checkpoint >= iter->second.size()) {
        logging::error(
            "No map registered for checkpoint {} of scene {}",
            checkpoint,
            scene.value()
        );
        return;
    }
    auto& map_info = iter->second.at(checkpoint);
    logging::trace("Map {}", map_info.map);
    stats.map = map_info.map;
    stats.map_stage = MapStage::main;  // always render stats
    if (stats.map > 0) {
        // spotted/evidence pointers are not working
        /*
        auto spotted = read<int32_t>(handle, base_ptrs[0] + 0xD61568);
        if (spotted) {
            stats.spotted = stats_value(
                spotted.value(), map_info.max_rating != Rating::unrated
            );
        }
        auto evidence_collected
            = read<int32_t>(handle, base_ptrs[0] + 0xE20FB0, {0x9C});
        if (evidence_collected) {
            stats.evidence_left = stats_value(
                map_info.num_evidence - evidence_collected.value(),
                map_info.max_rating != Rating::unrated
                    && map_info.max_rating != Rating::silent_assassin
            );
        }
        auto status = get_rating_status(map_info.max_rating, stats);
        stats.rating = {get_rating_value(map_info.max_rating, status), status};
        */
    }
}

constexpr float time_scale = 1.0f / (1024 * 1024);

void hitman_absolution::update_fast(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    if (stats.map > 0) {
        auto time = read<GameTimeManager>(handle, base_ptrs[0] + 0xE24730);
        if (time) {
            stats.time = time.value().game_time * time_scale;
        } else {
            logging::error("Unable to read time");
        }
    }
}
