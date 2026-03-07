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
    int num_objectives;
    Rating max_rating;
};

// level, checkpoint -> map info
const std::vector<std::vector<MapInfo>> scenes = {
    // level 0
    {
        {1},                        // 0 garden
        {2},                        // 1 greenhouse
        {3, 1},                     // 2 cliffside
        {4, 1, 2, Rating::shadow},  // 3 mansion ground
        {5},                        // 4 mansion 2nd
        {},                         // 5 level summary screen
    },
    // level 1
    {
        {6, 1, 2, Rating::silent_assassin},  // 0 king of chinatown
        {},                                  // 1 level summary screen
    },
    // level 2
    {
        {7, 1, 1, Rating::shadow},  // 0 terminus hotel
        {},                         // 1 elevator going up (cutscene)
        {8, 1, 2, Rating::shadow},  // 2 upper floors
        {9},                        // 3 room 899
        {},                         // 4 level summary screen
    },
    // level 3
    {
        {10, 0, 1, Rating::specialist},  // 0 burning hotel
        {11, 1, 1, Rating::shadow},      // 1 library
        {12},                            // 2 pigeon coop
        {13},                            // 3 rooftops
        {14, 1, 1, Rating::shadow},      // 4 shangri-la
        {},                              // 5 elevator going down (cutscene)
        {15, 1, 3, Rating::shadow},      // 6 train station
        {},                              // 7 level summary screen
    },
    // level 4
    {
        {16, 1, 1, Rating::shadow},           // 0 courtyard
        {17, 1, 3, Rating::silent_assassin},  // 1 vixen club
        {18, 0, 1, Rating::specialist},       // 2 dressing rooms
        {19, 1, 1, Rating::shadow},           // 3 derelict building
        {20, 1, 1, Rating::shadow},           // 4 convenience store
        {21, 1},                              // 5 loading area
        {22, 1, 4, Rating::silent_assassin},  // 6 chinese new year
        {},                                   // 7 lock pick (cutscene)
        {},                                   // 8 level summary screen
    },
    // level 5
    {},
    // level 6
    {
        {23},                                 // 0 victoria's ward
        {24, 1, 5, Rating::shadow},           // 1 orphanage halls
        {},                                   // 2 lenny shoots nun (cutscene)
        {25, 1, 2, Rating::silent_assassin},  // 3 central heating
        {},                                   // 4 level summary screen
    },
    // level 7
    {
        {26, 1, 1, Rating::shadow},  // 0 great balls of fire
        {},                          // 1 level summary screen
    },
    // level 8
    {
        {27, 1, 2, Rating::veteran},  // 0 gunshop
        {},                           // 1 level summary screen
    },
    // level 9
    {
        {28, 1, 4, Rating::silent_assassin},  // 0 streets of hope
        {29, 1, 3, Rating::silent_assassin},  // 1 barber shop
        {},                                   // 2 lenny kidnap (cutscene)
        {},                                   // 3 level summary screen
    },
    // level 10
    {
        {30},  // 0 the desert
        {},    // 1 level summary screen
    },
    // level 11
    {
        {},                          // 0 dog & oil fields (cutscene)
        {31, 1, 3, Rating::shadow},  // 1 dead end
        {32, 1, 1, Rating::shadow},  // 2 old mill
        {33, 1, 1, Rating::shadow},  // 3 descent
        {34, 1, 3, Rating::shadow},  // 4 factory compound
        {},                          // 5 level summary screen
    },
    // level 12
    {
        {35, 1, 5, Rating::silent_assassin},  // 0 test facility
        {36, 1, 2, Rating::shadow},           // 1 decontamination
        {37, 1, 3, Rating::silent_assassin},  // 2 r&d
        {},                                   // 3 level summary screen
    },
    // level 13
    {
        {38, 1, 1, Rating::shadow},           // 0 patriot's hangar
        {39, 1, 2, Rating::silent_assassin},  // 1 arena
        {},                                   // 2 level summary screen
    },
    // level 14
    {
        {40, 1, 3, Rating::silent_assassin},  // 0 parking
        {41, 1, 3, Rating::silent_assassin},  // 1 reception
        {42, 1, 4, Rating::silent_assassin},  // 2 cornfield
        {},                                   // 3 cutscene
        {},                                   // 4 level summary screen
    },
    // level 15
    {},
    // level 16
    {},
    // level 17
    {
        {43, 1, 1, Rating::shadow},  // 0 courthouse
        {44, 1},                     // 1 holding cells
        {45, 1, 2, Rating::shadow},  // 2 prison
        {},                          // 3 electrocution cutscene
        {},                          // 4 level summary screen
    },
    // level 18
    {
        {46, 1, 2, Rating::shadow},  // 0 county jail
        {47, 1, 1, Rating::shadow},  // 1 outgunned
        {48, 1, 1, Rating::shadow},  // 2 burn
        {49, 1, 1, Rating::shadow},  // 3 hope fair
        {50},                        // 4 outskirts
        {51},                        // 5 church
        {},                          // 6 level summary screen
    },
    // level 19
    {},
    // level 20
    {},
    // level 21
    {
        {52, 1, 1},  // 0 tailor shop
        {},          // 1 level summary screen
    },
    // level 22
    {
        {53, 1, 2, Rating::shadow},           // 0 blackwater park
        {},                                   // 1 sushi guy cutscene
        {54, 1, 3, Rating::silent_assassin},  // 2 the penthouse
        {},                                   // 3 level summary screen
    },
    // level 23
    {},
    // level 24
    {
        {55, 1, 1, Rating::silent_assassin},  // 0 blackwater roof
        {},                                   // 1 level summary screen
    },
    // level 25
    {
        {56, 1, 1, Rating::shadow},           // 0 cemetary entrance
        {57, 1, 3, Rating::silent_assassin},  // 1 burnwood family tomb
        {58, 1, 5, Rating::silent_assassin},  // 2 crematorium
        {},                                   // 3 level summary screen
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

constexpr int32_t NUM_DIFFICULTIES = 5;
// note: not all levels are used
constexpr int32_t NUM_LEVELS = 26;
// levels have fewer than 13 checkpoints
// the 13 is an upper bound from the engine, used in the array of stats values
constexpr int32_t NUM_CHECKPOINTS_PER_LEVEL = 13;
constexpr int32_t MAX_CHALLENGES = 278;

// manages checkpoints for the currently loaded level
struct CheckpointManager {
    int8_t _pad0[0x24];
    int32_t _unknown24;       // +0x24
    int32_t checkpoints_ptr;  // +0x28
};

static_assert(offsetof(CheckpointManager, checkpoints_ptr) == 0x28);

// stores checkpoint entries (each 8 bytes, see CheckpointEntry)
// for currently loaded level
// along with the key of the currently active checkpoint
struct Checkpoints {
    int8_t _pad0[0x0C];
    int32_t entry_begin_ptr;  // +0x0C first CheckpointEntry
    int32_t entry_end_ptr;    // +0x10 past last CheckpointEntry
    int8_t _pad1[0x3C];       // ...
    int32_t current_key;      // +0x50
};

static_assert(offsetof(Checkpoints, entry_begin_ptr) == 0x0C);
static_assert(offsetof(Checkpoints, entry_end_ptr) == 0x10);
static_assert(offsetof(Checkpoints, current_key) == 0x50);

// stores a unique key per checkpoint
struct CheckpointEntry {
    int32_t key;
    int32_t _unknown;
};

static_assert(sizeof(CheckpointEntry) == 0x08);
static_assert(offsetof(CheckpointEntry, key) == 0x00);

struct StatsManager {
    int8_t _pad0[4];
    int32_t entry_begin_ptr;  // 04
    int32_t entry_end_ptr;    // 08
    int8_t _pad1[0x1C];       // ...
    int32_t values_ptr;       // 28
    int8_t _pad2[0x54];       // ...
    int32_t score;            // 80
};

static_assert(offsetof(StatsManager, entry_begin_ptr) == 0x04);
static_assert(offsetof(StatsManager, entry_end_ptr) == 0x08);
static_assert(offsetof(StatsManager, values_ptr) == 0x28);
static_assert(offsetof(StatsManager, score) == 0x80);

struct GameStats {
    int16_t unknown;                // 00
    int16_t objective_complete;     // 02
    int16_t target_kill;            // 04
    int16_t spotted;                // 06
    int16_t evidence_removed;       // 08
    int16_t silent_assassin_bonus;  // 0A
    int16_t signature_kill;         // 0C
    int16_t silent_kill;            // 0E
    int16_t headshot;               // 10
    int16_t body_hidden;            // 12
    int16_t civilian_casualty;      // 14
    int16_t non_target_casualty;    // 16
    int16_t pacification;           // 18
    int16_t _unknown[87];           // 1A
};

static_assert(sizeof(GameStats) == 200);

static constexpr std::array<int32_t, 13> STATS_MULTIPLIERS = {
    0,      // 00 unknown
    5000,   // 01 objective_complete
    10000,  // 02 target_kill
    -1000,  // 03 spotted
    1000,   // 04 evidence_removed
    47000,  // 05 silent_assassin_bonus
    5000,   // 06 signature_kill
    150,    // 07 silent_kill
    150,    // 08 headshot
    100,    // 09 body_hidden
    -2500,  // 10 civilian_casualty
    -250,   // 11 non_target_casualty
    -100,   // 12 pacification
};

static int32_t get_difficulty(void* handle, const BasePtrs& base_ptrs) {
    auto difficulty = read<int32_t>(handle, base_ptrs[0] + 0xD58D04);
    if (!difficulty) {
        logging::error("Unable to read difficulty");
        return -1;
    }
    auto value = difficulty.value();
    if (value < 0 || value >= NUM_DIFFICULTIES) {
        logging::error("Difficulty {} out of bounds", value);
        return -1;
    }
    return value;
}

static int32_t get_level(void* handle, const BasePtrs& base_ptrs) {
    auto level = read<int32_t>(handle, base_ptrs[0] + 0xE21394);
    if (!level) {
        logging::error("Unable to read level");
        return -1;
    }
    auto value = level.value();
    // engine may set level to -1 if not in a mission
    // sadly it's not a reliable way to detect if we are in a mission
    if (value < -1 || value >= NUM_LEVELS) {
        logging::error("Level {} out of bounds", value);
        return -1;
    }
    return value;
}

static std::optional<CheckpointManager> get_checkpoint_manager(
    void* handle, const BasePtrs& base_ptrs
) {
    return read<CheckpointManager>(handle, base_ptrs[0] + 0xE21580);
}

static std::optional<Checkpoints> get_checkpoints(
    void* handle, int32_t checkpoints_ptr
) {
    return read<Checkpoints>(handle, checkpoints_ptr);
}

static std::optional<CheckpointEntry> get_checkpoint_entry(
    void* handle, int32_t entry_ptr
) {
    return read<CheckpointEntry>(handle, entry_ptr);
}

// replicate game engine logic for calculating checkpoint
static int32_t get_current_checkpoint_index(
    void* handle, const Checkpoints& checkpoints
) {
    if (checkpoints.current_key == 0) {
        // level is loading but we have not started yet
        logging::trace("Current checkpoint key is null");
        return -1;
    }
    int32_t index = 0;
    auto entry_ptr = checkpoints.entry_begin_ptr;
    // note: upper bound on index to avoid infinite loop on corrupt data
    while (entry_ptr != checkpoints.entry_end_ptr
           && index < NUM_CHECKPOINTS_PER_LEVEL) {
        auto entry = get_checkpoint_entry(handle, entry_ptr);
        if (!entry) {
            logging::error("Unable to read checkpoint entry");
            return -1;
        }
        if (checkpoints.current_key == entry->key) return index;
        entry_ptr += sizeof(CheckpointEntry);
        index++;
    }
    logging::error("Unable to find current checkpoint key");
    return -1;
}

void hitman_absolution::update_slow(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    auto difficulty = get_difficulty(handle, base_ptrs);
    if (difficulty >= 0) stats.difficulty = difficulty;
    auto level = get_level(handle, base_ptrs);
    if (level < 0) {
        stats.map = 0;
        return;
    }
    const auto checkpoint_manager = get_checkpoint_manager(handle, base_ptrs);
    if (!checkpoint_manager) {
        logging::error("Unable to read checkpoint manager");
        return;
    }
    if (checkpoint_manager->checkpoints_ptr == 0) {
        // in main menu and haven't loaded level yet
        logging::trace("Checkpoints pointer is null");
        return;
    }
    const auto checkpoints
        = get_checkpoints(handle, checkpoint_manager->checkpoints_ptr);
    if (!checkpoints) {
        logging::error("Unable to read checkpoints");
        return;
    }
    auto checkpoint_index = get_current_checkpoint_index(handle, *checkpoints);
    if (checkpoint_index < 0) {
        stats.map = 0;
        return;
    }
    logging::trace("Level {} at checkpoint {}", level, checkpoint_index);
    if (level >= scenes.size()) {
        logging::error("No map registered for level {}", level);
        return;
    }
    auto& map_infos = scenes.at(level);
    if (checkpoint_index >= map_infos.size()) {
        logging::error(
            "No map registered for level {}, checkpoint {}",
            level,
            checkpoint_index
        );
        return;
    }
    auto& map_info = map_infos.at(checkpoint_index);
    logging::trace("Map {}", map_info.map);
    stats.map = map_info.map;
    stats.map_stage = MapStage::main;  // always render stats
    if (stats.map > 0) {
        auto stats_manager
            = read<StatsManager>(handle, base_ptrs[0] + 0xD61710);
        if (!stats_manager) {
            logging::error("Unable to read stats manager");
            return;
        }
        auto game_stats = read<GameStats>(
            handle,
            stats_manager->values_ptr
                + ((level * NUM_CHECKPOINTS_PER_LEVEL + checkpoint_index) * 200)
        );
        if (!game_stats) {
            logging::error("Unable to read game stats");
            return;
        }
        // note: stats are always 0 for unrated maps
        stats.innocents_killed = stats_value(game_stats->civilian_casualty);
        stats.enemies_killed = stats_value(
            game_stats->non_target_casualty - game_stats->civilian_casualty,
            // maps without silent assassin rating allow enemies killed
            map_info.max_rating == Rating::silent_assassin
        );
        stats.alerts = stats_value(game_stats->spotted);
        stats.on_camera
            = {map_info.num_evidence - game_stats->evidence_removed};
        stats.objectives_left
            = {map_info.num_objectives - game_stats->objective_complete};
        stats.pacifications = {game_stats->pacification};
        stats.bodies_hidden = {game_stats->body_hidden};
        stats.headshots = {game_stats->headshot};
        stats.silent_kills = {game_stats->silent_kill};
        stats.signature_kills = {game_stats->signature_kill};
        stats.sa_bonus = {game_stats->silent_assassin_bonus};
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
