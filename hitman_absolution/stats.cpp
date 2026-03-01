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

enum class Rating { unrated, veteran, specialist, shadow, silent_assassin };

struct MapInfo {
    int map;
    int num_evidence;
    Rating max_rating;
};

// level, checkpoint -> map info
const std::vector<std::vector<MapInfo>> scenes = {
    // level 0
    {
        {1},                     // 0 garden
        {2},                     // 1 greenhouse
        {3, 1},                  // 2 cliffside
        {4, 1, Rating::shadow},  // 3 mansion ground
        {5},                     // 4 mansion 2nd
    },
    // level 1
    {
        {6, 1, Rating::silent_assassin},  // 0 king of chinatown
        {},                               // 1 cutscene
    },
    // level 2
    {
        {7, 1, Rating::shadow},  // 0 terminus hotel
        {8, 1, Rating::shadow},  // 1 upper floors (elevator cutscene)
        {8, 1, Rating::shadow},  // 2 upper floors
        {8, 1, Rating::shadow},  // 3 upper floors (after picking the lock)
    },
    // level 3
    {
        {9, 0, Rating::specialist},  // 0 burning hotel
        {10, 1, Rating::shadow},     // 1 library
        {11, 0},                     // 2 pigeon coop (part with heli)
        {11, 0},                     // 3 pigeon coop (part with cops)
        {12, 1, Rating::shadow},     // 4 shangri-la
        {13, 1, Rating::shadow},     // 5 train station (elevator cutscene)
        {13, 1, Rating::shadow},     // 6 train station
    },
    // level 4
    {
        {14, 1, Rating::shadow},           // 0 courtyard
        {15, 1, Rating::silent_assassin},  // 1 vixen club
        {16, 0, Rating::specialist},       // 2 dressing rooms
        {17, 1, Rating::shadow},           // 3 derelict building
        {18, 1, Rating::shadow},           // 4 convenience store
        {19, 1},                           // 5 loading area
        {20, 1, Rating::silent_assassin},  // 6 chinese new year
        {20, 1, Rating::silent_assassin},  // 7 chinese new year (lock pick)
    },
    // level 5
    {},
    // level 6
    {
        {21, 0},                           // 0 victoria's ward
        {22, 1, Rating::shadow},           // 1 orphanage halls
        {23, 1, Rating::silent_assassin},  // 2 central heating (cutscene)
        {23, 1, Rating::silent_assassin},  // 3 central heating
    },
    // level 7
    {
        {24, 1, Rating::shadow},  // 0 great balls of fire
    },
    // level 8
    {
        {25, 1, Rating::veteran},  // 0 gunshop
    },
    // level 9
    {
        {26, 1, Rating::silent_assassin},  // 0 streets of hope
        {27, 1, Rating::silent_assassin},  // 1 barber shop
        {27, 1, Rating::silent_assassin},  // 2 barber shop (cutscene)
    },
    // level 10
    {
        {28, 0},  // 0 the desert
    },
    // level 11
    {
        {29, 1, Rating::shadow},  // 0 dead end (loading)
        {29, 1, Rating::shadow},  // 1 dead end
        {30, 1, Rating::shadow},  // 2 old mill
        {31, 1, Rating::shadow},  // 3 descent
        {32, 1, Rating::shadow},  // 4 factory compound
    },
    // level 12
    {
        {33, 1, Rating::silent_assassin},  // 0 test facility
        {34, 1, Rating::shadow},           // 1 decontamination
        {35, 1, Rating::silent_assassin},  // 2 r&d
    },
    // level 13
    {
        {36, 1, Rating::shadow},           // 0 patriot's hangar
        {37, 1, Rating::silent_assassin},  // 1 arena
    },
    // level 14
    {
        {38, 1, Rating::silent_assassin},  // 0 parking
        {39, 1, Rating::silent_assassin},  // 1 reception
        {40, 1, Rating::silent_assassin},  // 2 cornfield
    },
    // level 15
    {},
    // level 16
    {},
    // level 17
    {
        {41, 1, Rating::shadow},  // 0 courthouse
        {42, 1},                  // 1 holding cells
        {43, 1, Rating::shadow},  // 2 prison
        {43, 1, Rating::shadow},  // 3 prison (cutscene)
    },
    // level 18
    {
        {44, 1, Rating::shadow},  // 0 county jail
        {45, 1, Rating::shadow},  // 1 outgunned
        {46, 1, Rating::shadow},  // 2 burn
        {47, 1, Rating::shadow},  // 3 hope fair
        {48, 0},                  // 4 church (outside)
        {48, 0},                  // 4 church (inside)
    },
    // level 19
    {},
    // level 20
    {},
    // level 21
    {
        {49, 1},  // 0 tailor shop
    },
    // level 22
    {
        {50, 1, Rating::shadow},           // 0 blackwater park
        {50, 1, Rating::shadow},           // 1 blackwater park
        {51, 1, Rating::silent_assassin},  // 2 the penthouse
    },
    // level 23
    {},
    // level 24
    {
        {52, 1, Rating::silent_assassin},  // 0 blackwater roof
    },
    // level 25
    {
        {53, 1, Rating::shadow},           // 0 cemetary entrance
        {54, 1, Rating::silent_assassin},  // 1 burnwood family tomb
        {55, 1, Rating::silent_assassin},  // 2 crematorium
    },
};

static Status get_rating_status(Rating max_rating, const Stats& stats) {
    if (max_rating == Rating::unrated) return Status::GREEN;
    return (stats.alerts.value != 0 || stats.innocents_killed.value != 0
            || (max_rating == Rating::silent_assassin
                && stats.enemies_killed.value != 0))
               ? Status::RED
               : Status::GREEN;
};

constexpr int32_t NUM_LEVELS = 26;
constexpr int32_t NUM_CHECKPOINTS_PER_LEVEL
    = 13;  // hard upper bound (seen in assembly code)

struct CheckpointManager {
    int8_t _pad0[0x24];
    int32_t unknown_24;                // +0x24
    int32_t checkpoint_container_ptr;  // +0x28
};

struct CheckpointContainer {
    int8_t _pad0[0x0C];
    int32_t keys_start_ptr;  // +0x0C points to entry[0].key
    int32_t keys_end_ptr;    // +0x10 points one past last entry
    int8_t _pad1[0x3C];      // ...
    int32_t current_key;     // +0x50
};

struct GameStats {
    int16_t unknown;  // silent assassin bonus?
    int16_t objective_complete;
    int16_t target_kill;
    int16_t spotted;
    int16_t evidence_removed;
    int16_t silent_assassin_bonus;
    int16_t signature_kill;
    int16_t silent_kill;
    int16_t headshot;
    int16_t body_hidden;
    int16_t civilian_casualty;
    int16_t non_target_casualty;
    int16_t pacification;
};

static int32_t get_level(void* handle, const BasePtrs& base_ptrs) {
    auto level = read<int32_t>(handle, base_ptrs[0] + 0xE21394);
    if (!level) {
        logging::error("Unable to read level");
        return -1;
    }
    auto level_value = level.value();
    // engine may set level to -1 if not in a mission
    // sadly it's not a reliable way to detect if we are in a mission
    if (level_value < -1 || level_value >= NUM_LEVELS) {
        logging::error("Level {} out of bounds", level_value);
        return -1;
    }
    return level_value;
}

// replicate game engine logic for calculating checkpoint
static int32_t get_checkpoint(void* handle, const BasePtrs& base_ptrs) {
    const auto manager
        = read<CheckpointManager>(handle, base_ptrs[0] + 0xE21580);
    if (!manager) {
        logging::error("Unable to read checkpoint manager");
        return -1;
    }
    if (manager->checkpoint_container_ptr == 0) {
        // in main menu and haven't loaded level yet
        logging::debug("Checkpoint container pointer is null");
        return -1;
    }
    const auto container
        = read<CheckpointContainer>(handle, manager->checkpoint_container_ptr);
    if (!container) {
        logging::error("Unable to read checkpoint container");
        return -1;
    }
    if (container->current_key == 0) {
        // level is loading but we have not started yet
        logging::debug("Current checkpoint key is null");
        return -1;
    }
    // each checkpoint entry is 8 bytes: check and calculate num_checkpoints
    const auto total_bytes
        = container->keys_end_ptr - container->keys_start_ptr;
    if ((total_bytes & 7) != 0) {
        logging::error(
            "Checkpoint table size {} not divisible by 8", total_bytes
        );
        return -1;
    }
    const auto num_checkpoints = total_bytes >> 3;
    logging::trace("Number of checkpoints is {}", num_checkpoints);
    if ((num_checkpoints <= 0)
        || (num_checkpoints > NUM_CHECKPOINTS_PER_LEVEL)) {
        logging::error(
            "Number of checkpoints {} out of range", num_checkpoints
        );
        return -1;
    };
    // look up current checkpoint key in the checkpoint keys table
    for (int32_t checkpoint = 0; checkpoint < num_checkpoints; checkpoint++) {
        auto key
            = read<int32_t>(handle, container->keys_start_ptr + checkpoint * 8);
        if (!key) {
            logging::error("Unable to read from checkpoint table");
            return -1;
        }
        if (key.value() == container->current_key) return checkpoint;
    };
    logging::error("Checkpoint key {:#x} not found", container->current_key);
    return -1;
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
    auto level = get_level(handle, base_ptrs);
    if (level < 0) {
        stats.map = 0;
        return;
    }
    auto checkpoint = get_checkpoint(handle, base_ptrs);
    if (checkpoint < 0) {
        stats.map = 0;
        return;
    }
    logging::trace("Level {} at checkpoint {}", level, checkpoint);
    if (level >= scenes.size()) {  // checked previously... but just in case
        logging::error("No map registered for level {}", level);
        return;
    }
    auto& map_infos = scenes.at(level);
    if (checkpoint >= map_infos.size()) {
        logging::error(
            "No map registered for level {}, checkpoint {}", level, checkpoint
        );
        return;
    }
    auto& map_info = map_infos.at(checkpoint);
    logging::trace("Map {}", map_info.map);
    stats.map = map_info.map;
    stats.map_stage = MapStage::main;  // always render stats
    if (stats.map > 0) {
        auto game_stats_ptr
            = read<int32_t>(handle, base_ptrs[0] + 0xD61710 + 0x28);
        if (!game_stats_ptr) {
            logging::error("Unable to read game stats pointer");
            return;
        }
        auto game_stats = read<GameStats>(
            handle,
            game_stats_ptr.value()
                + ((level * NUM_CHECKPOINTS_PER_LEVEL + checkpoint) * 200)
        );
        if (!game_stats) {
            logging::error("Unable to read game stats");
            return;
        }
        // note: stats are not shown for unrated maps
        // so we don't bother about individual stats rating in this case
        stats.innocents_killed = stats_value(game_stats->civilian_casualty);
        stats.enemies_killed = stats_value(
            game_stats->non_target_casualty - game_stats->civilian_casualty,
            // maps without silent assassin rating allow enemies killed
            map_info.max_rating == Rating::silent_assassin
        );
        stats.alerts = stats_value(game_stats->spotted);
        stats.on_camera = stats_value(
            map_info.num_evidence - game_stats->evidence_removed, false
        );
        auto status = get_rating_status(map_info.max_rating, stats);
        stats.rating
            = {map_info.max_rating == Rating::unrated
                   ? "Unrated"
                   : get_simple_rating_value(status),
               status};
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
