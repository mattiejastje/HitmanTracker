#include "stats.hpp"

#include <format>
#include <iostream>  // std::cout
#include <mempeep/read.hpp>
#include <mempeep/tracers/log_tracer.hpp>
#include <mempeep/tracers/ok_tracer.hpp>
#include <unordered_map>

#include "../hitman_common/stats.hpp"
#include "../logging.hpp"
#include "../mem/read_write.hpp"
#include "structs.hpp"

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
            || stats.enemies_killed.value != 0)
               ? Status::RED
               : Status::GREEN;
};

constexpr std::size_t OBJECTIVE_COMPLETE = 1;
constexpr std::size_t TARGET_KILL = 2;
constexpr std::size_t SPOTTED = 3;
constexpr std::size_t EVIDENCE_REMOVED = 4;
constexpr std::size_t SILENT_ASSASSIN_BONUS = 5;
constexpr std::size_t SIGNATURE_KILL = 6;
constexpr std::size_t SILENT_KILL = 7;
constexpr std::size_t HEADSHOT = 8;
constexpr std::size_t BODY_HIDDEN = 9;
constexpr std::size_t CIVILIAN_CASUALTY = 10;
constexpr std::size_t NON_TARGET_CASUALTY = 11;
constexpr std::size_t PACIFICATION = 12;

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

// replicate game engine logic for calculating checkpoint
static int32_t get_current_checkpoint_index(
    const hitman_absolution::structs::Checkpoints& checkpoints
) {
    if (checkpoints.current_key == 0) {
        // level is loading but we have not started yet
        logging::trace("Current checkpoint key is null");
        return -1;
    }
    int32_t index = 0;
    for (auto& checkpoint : checkpoints.checkpoint) {
        if (checkpoints.current_key == checkpoint.key) return index;
        index++;
    }
    logging::error("Unable to find current checkpoint key");
    return -1;
}

int32_t get_raw_score(const std::array<int16_t, 0x64>& values) {
    int32_t score = 0;
    for (std::size_t index = 1; index < 13; index++) {
        score += values[index] * STATS_MULTIPLIERS[index];
    }
    return score;
};

const hitman_absolution::structs::GameDataLevelInfo* get_level_info(
    const std::vector<hitman_absolution::structs::GameDataLevelInfo>&
        level_infos,
    int32_t level
) {
    for (const auto& level_info : level_infos)
        if (level_info.level_data.level == level) return &level_info;
    return nullptr;
}

// TODO hardcode instead of calculate so we don't need to read the data
std::optional<int32_t> get_best_raw_score(
    const std::vector<hitman_absolution::structs::GameDataLevelInfo>&
        level_infos,
    int32_t level,
    int32_t checkpoint_index
) {
    auto level_info = get_level_info(level_infos, level);
    if (level_info) {
        auto& checkpoint_info = level_info->checkpoint_info;
        if (checkpoint_info) {
            auto& nodes = checkpoint_info->nodes;
            if (0 <= checkpoint_index && checkpoint_index < nodes.size()) {
                auto& node = checkpoint_info->nodes[checkpoint_index];
                auto& data = node.data;
                if (data) return data->best_raw_score;
            }
        }
    }
    return {};
}

// global to avoid allocating large object on stack
static hitman_absolution::structs::Game game{};

bool hitman_absolution::update_slow(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats,
    float dt
) {
    MemoryReader<uint32_t> reader{handle};
    // auto tracer = mempeep::OkTracer{};
    // TODO check performance against OkTracer
    auto tracer
        = mempeep::LogTracer{MempeepOnLogEntry{}, mempeep::LogLevel::VALUES};
    if (!mempeep::read<structs::TGame>(base_ptrs[0], reader, tracer, game))
        return false;
    stats.difficulty = game.difficulty;
    // engine may set level to -1 if not in a mission
    // sadly it's not a reliable way to detect if we are in a mission
    if (game.level == -1) {
        stats.map = 0;
        return true;
    }
    if (!game.checkpoints_manager.checkpoints) {
        // in main menu and haven't loaded level yet
        logging::trace("Checkpoints pointer is null");
        return true;
    }
    auto checkpoints = *game.checkpoints_manager.checkpoints;
    auto checkpoint_index = get_current_checkpoint_index(checkpoints);
    if (checkpoint_index < 0) {
        stats.map = 0;
        return true;
    }
    logging::trace("Level {} at checkpoint {}", game.level, checkpoint_index);
    if (game.level >= scenes.size()) {
        logging::error("No map registered for level {}", game.level);
        return false;
    }
    auto& map_infos = scenes.at(game.level);
    if (checkpoint_index >= map_infos.size()) {
        logging::error(
            "No map registered for level {}, checkpoint {}",
            game.level,
            checkpoint_index
        );
        return false;
    }
    auto& map_info = map_infos.at(checkpoint_index);
    logging::trace("Map {}", map_info.map);
    stats.map = map_info.map;
    stats.map_stage = MapStage::main;  // always render stats
    if (stats.map > 0) {
        auto& stats_manager = game.stats_manager;
        auto& game_stats = stats_manager.values[game.level][checkpoint_index];
        // note: stats are always 0 for unrated maps
        stats.innocents_killed = stats_value(game_stats[CIVILIAN_CASUALTY]);
        stats.enemies_killed = stats_value(
            game_stats[NON_TARGET_CASUALTY] - game_stats[CIVILIAN_CASUALTY]
        );
        stats.alerts = stats_value(game_stats[SPOTTED]);
        auto status = get_rating_status(map_info.max_rating, stats);
        stats.rating
            = {map_info.max_rating == Rating::unrated
                   ? "Unrated"
                   : get_simple_rating_value(status),
               status};
        auto score = get_raw_score(game_stats);
        auto score_shadow = get_best_raw_score(
            game.game_data.level_infos, game.level, checkpoint_index
        );
        if (score_shadow && score_shadow.value() != 0) {
            int32_t percent = (100 * score) / score_shadow.value();
            auto score_status = percent < 100 ? Status::YELLOW : Status ::GREEN;
            stats.score = {score, score_status};
            stats.score_shadow = score_shadow.value();
            stats.score_rating
                = {percent < 50    ? "Agent"
                   : percent < 80  ? "Veteran"
                   : percent < 90  ? "Specialist"
                   : percent < 100 ? "Professional"
                                   : "Shadow",
                   score_status};
            stats.score_objective_complete
                = {STATS_MULTIPLIERS[OBJECTIVE_COMPLETE]
                       * game_stats[OBJECTIVE_COMPLETE],
                   game_stats[OBJECTIVE_COMPLETE] < map_info.num_objectives
                       ? Status::YELLOW
                       : Status::GREEN};
            stats.score_target_kill
                = {STATS_MULTIPLIERS[TARGET_KILL] * game_stats[TARGET_KILL]};
            stats.score_spotted
                = {STATS_MULTIPLIERS[SPOTTED] * game_stats[SPOTTED],
                   game_stats[SPOTTED] ? Status::RED : Status::GREEN};
            stats.score_evidence_removed
                = {STATS_MULTIPLIERS[EVIDENCE_REMOVED]
                       * game_stats[EVIDENCE_REMOVED],
                   game_stats[EVIDENCE_REMOVED] < map_info.num_evidence
                       ? Status::YELLOW
                       : Status::GREEN};
            stats.score_silent_assassin_bonus
                = {STATS_MULTIPLIERS[SILENT_ASSASSIN_BONUS]
                   * game_stats[SILENT_ASSASSIN_BONUS]};
            stats.score_signature_kill
                = {STATS_MULTIPLIERS[SIGNATURE_KILL]
                   * game_stats[SIGNATURE_KILL]};
            stats.score_silent_kill
                = {STATS_MULTIPLIERS[SILENT_KILL] * game_stats[SILENT_KILL]};
            stats.score_headshot
                = {STATS_MULTIPLIERS[HEADSHOT] * game_stats[HEADSHOT]};
            stats.score_body_hidden
                = {STATS_MULTIPLIERS[BODY_HIDDEN] * game_stats[BODY_HIDDEN]};
            stats.score_civilian_casualty
                = {STATS_MULTIPLIERS[CIVILIAN_CASUALTY]
                       * game_stats[CIVILIAN_CASUALTY],
                   game_stats[CIVILIAN_CASUALTY] ? Status::RED : Status::GREEN};
            stats.score_non_target_casualty
                = {STATS_MULTIPLIERS[NON_TARGET_CASUALTY]
                       * game_stats[NON_TARGET_CASUALTY],
                   game_stats[NON_TARGET_CASUALTY] ? Status::RED
                                                   : Status::GREEN};
            stats.score_pacification
                = {STATS_MULTIPLIERS[PACIFICATION] * game_stats[PACIFICATION]};
        }
    }
    return true;
}

constexpr float time_scale = 1.0f / (1024 * 1024);

bool hitman_absolution::update_fast(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats,
    float dt
) {
    if (stats.map > 0) {
        auto game_time = read<int64_t>(handle, base_ptrs[0] + 0xE24730 + 0x18);
        if (game_time) stats.time = game_time.value() * time_scale;
        return game_time.has_value();
    }
    return true;
}
