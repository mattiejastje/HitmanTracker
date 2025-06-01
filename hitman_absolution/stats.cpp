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
    int32_t checkpoint_index;
    int8_t is_restoring;
    int8_t use_savegame;
};

static int32_t map_key(int32_t level, int32_t section) {
    return (level << 8) + section;
}

enum class Rating { unrated, veteran, specialist, shadow, silent_assassin };

struct MapInfo {
    int map;
    int num_evidence;
    Rating max_rating;
};

const std::unordered_map<int32_t, MapInfo> scenes = {
    {map_key(-1, -1), {0}},                            // game loading
    {map_key(0, 0), {1, 0}},                           // garden
    {map_key(0, 1), {2, 0}},                           // greenhouse
    {map_key(0, 2), {3, 1}},                           // cliffside
    {map_key(0, 3), {4, 1, Rating::shadow}},           // mansion ground
    {map_key(0, 4), {5, 0}},                           // mansion 2nd
    {map_key(1, 0), {6, 1, Rating::silent_assassin}},  // king of chinatown
    {map_key(2, 0), {7, 1, Rating::shadow}},           // terminus hotel
    {map_key(2, 1), {8, 1, Rating::shadow}},           // upper floors
    // TODO (2, 2)
    {map_key(3, 0), {9, 0, Rating::specialist}},        // burning hotel
    {map_key(3, 1), {10, 1, Rating::shadow}},           // library
    {map_key(3, 3), {11, 0}},                           // pigeon coop
    {map_key(3, 4), {12, 1, Rating::shadow}},           // shangri-la
    {map_key(3, 5), {13, 1, Rating::shadow}},           // train station
    {map_key(4, 0), {14, 1, Rating::shadow}},           // courtyard
    {map_key(4, 1), {15, 1, Rating::silent_assassin}},  // vixen club
    {map_key(4, 2), {16, 0, Rating::specialist}},       // dressing rooms
    {map_key(4, 3), {17, 1, Rating::shadow}},           // derelict building
    {map_key(4, 4), {18, 1, Rating::shadow}},           // convenience store
    {map_key(4, 5), {19, 1}},                           // loading area
    {map_key(4, 6), {20, 1, Rating::silent_assassin}},  // chinese new year
    {map_key(6, 0), {21, 0}},                           // victoria's ward
    {map_key(6, 1), {22, 1, Rating::shadow}},           // orphanage halls
    {map_key(6, 2), {0}},  // "sister mary" cutscene
    {map_key(6, 3), {23, 1, Rating::silent_assassin}},   // central heating
    {map_key(7, 0), {24, 1, Rating::shadow}},            // great balls of fire
    {map_key(8, 0), {25, 1, Rating::veteran}},           // gunshop
    {map_key(9, 0), {26, 1, Rating::silent_assassin}},   // streets of hope
    {map_key(9, 1), {27, 1, Rating::silent_assassin}},   // barber shop
    {map_key(10, 0), {28, 0}},                           // the desert
    {map_key(11, 0), {29, 1, Rating::shadow}},           // dead end
    {map_key(11, 1), {30, 1, Rating::shadow}},           // old mill
    {map_key(11, 2), {31, 1, Rating::shadow}},           // descent
    {map_key(11, 3), {32, 1, Rating::shadow}},           // factory compound
    {map_key(12, 0), {33, 1, Rating::silent_assassin}},  // test facility
    {map_key(12, 1), {34, 1, Rating::shadow}},           // decontamination
    {map_key(12, 2), {35, 1, Rating::silent_assassin}},  // r&d
    {map_key(13, 0), {36, 1, Rating::shadow}},           // patriot's hangar
    {map_key(13, 1), {37, 1, Rating::silent_assassin}},  // arena
    {map_key(14, 0), {38, 1, Rating::silent_assassin}},  // parking
    {map_key(14, 1), {39, 1, Rating::silent_assassin}},  // reception
    {map_key(14, 2), {40, 1, Rating::silent_assassin}},  // cornfield
    {map_key(17, 0), {41, 1, Rating::shadow}},           // courthouse
    {map_key(17, 1), {42, 1}},                           // holding cells
    {map_key(17, 2), {43, 1, Rating::shadow}},           // prison
    {map_key(18, 0), {44, 1, Rating::shadow}},           // county jail
    {map_key(18, 1), {45, 1, Rating::shadow}},           // outgunned
    {map_key(18, 2), {46, 1, Rating::shadow}},           // burn
    {map_key(18, 3), {47, 1, Rating::shadow}},           // hope fair
    {map_key(18, 4), {48, 0}},                           // church
    {map_key(21, 0), {49, 1}},                           // tailor shop
    {map_key(22, 1), {50, 1, Rating::shadow}},           // blackwater park
    {map_key(22, 2), {51, 1, Rating::silent_assassin}},  // the penthouse
    {map_key(24, 0), {52, 1, Rating::silent_assassin}},  // blackwater roof
    {map_key(25, 0), {53, 1, Rating::shadow}},           // cemetary entrance
    {map_key(25, 1), {54, 1, Rating::silent_assassin}},  // burnwood family tomb
    {map_key(25, 2), {55, 1, Rating::silent_assassin}},  // crematorium
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
    // TODO use scene & level_manager.checkpoint_index instead of level/section
    auto level = read<int32_t>(handle, base_ptrs[0] + 0xE20F48);
    auto section = read<int32_t>(handle, base_ptrs[0] + 0xD60F94);
    if (!level) {
        logging::error("Unable to read level");
        return;
    }
    if (!section) {
        logging::error("Unable to read section");
        return;
    }
    auto key = map_key(level.value(), section.value());
    logging::trace(
        "Scene {}, level {}, section {}, key {}",
        scene.value(),
        level.value(),
        section.value(),
        key
    );
    auto iter = scenes.find(key);
    if (iter != scenes.end()) {
        stats.map = iter->second.map;
        logging::trace("Map {}", stats.map);
    } else {
        logging::error(
            "No map registered for level {} section {}",
            level.value(),
            section.value()
        );
        return;
    }
    stats.map_stage = MapStage::main;  // always render stats
    if (stats.map > 0) {
        auto spotted = read<int32_t>(handle, base_ptrs[0] + 0xD61568);
        if (spotted) {
            stats.spotted = stats_value(
                spotted.value(), iter->second.max_rating != Rating::unrated
            );
        }
        auto evidence_collected
            = read<int32_t>(handle, base_ptrs[0] + 0xE20FB0, {0x9C});
        if (evidence_collected) {
            stats.evidence_left = stats_value(
                iter->second.num_evidence - evidence_collected.value(),
                iter->second.max_rating != Rating::unrated
                    && iter->second.max_rating != Rating::silent_assassin
            );
        }
        auto status = get_rating_status(iter->second.max_rating, stats);
        stats.rating
            = {get_rating_value(iter->second.max_rating, status), status};
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
