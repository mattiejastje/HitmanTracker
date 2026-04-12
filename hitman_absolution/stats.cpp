#include "stats.hpp"

#include <format>
#include <iostream>  // std::cout
#include <mempeep/read.hpp>
#include <mempeep/tracers/log_tracer.hpp>

#include "../hitman_common/stats.hpp"
#include "../logging.hpp"
#include "../mem/read_write.hpp"
#include "structs.hpp"

constexpr int AGENT = 0;
constexpr int VETERAN = 50;
constexpr int SPECIALIST = 80;
constexpr int PROFESSIONAL = 90;
constexpr int SHADOW = 100;

struct MapInfo {
    int map;
    int num_evidence;
    int num_objectives;
    // 0 = agent, 50 = veteran, 80 = specialist, 90 = professional, 100 = shadow
    int max_rating;
    int num_targets;
};

// level, checkpoint -> map info
const std::vector<std::vector<MapInfo>> scenes = {
    // level 0
    {
        // 0 garden
        {1},
        // 1 greenhouse
        {2},
        // 2 cliffside
        {3, 1},
        // 3 mansion ground
        {4, 1, 2, SHADOW},
        // 4 mansion 2nd
        {5},
        // 5 level summary screen
        {},
    },
    // level 1
    {
        // 0 king of chinatown
        {6, 1, 3, SHADOW, 1},
        // 1 level summary screen
        {},
    },
    // level 2
    {
        // 0 terminus hotel
        {7, 1, 1, SHADOW},
        // 1 elevator going up (cutscene)
        {},
        // 2 upper floors
        {8, 1, 2, SHADOW},
        // 3 room 899
        {9},
        // 4 level summary screen
        {},
    },
    // level 3
    {
        // 0 burning hotel
        {10, 0, 1, SPECIALIST},
        // 1 library
        {11, 1, 1, SHADOW},
        // 2 pigeon coop
        {12},
        // 3 rooftops
        {13},
        // 4 shangri-la
        {14, 1, 1, SHADOW},
        // 5 elevator going down (cutscene)
        {},
        // 6 train station
        {15, 1, 3, SHADOW},
        // 7 level summary screen
        {},
    },
    // level 4
    {
        // 0 courtyard
        {16, 1, 1, SHADOW},
        // 1 vixen club
        {17, 1, 3, SHADOW, 1},
        // 2 dressing rooms
        {18, 0, 1, SPECIALIST},
        // 3 derelict building
        {19, 1, 1, SHADOW},
        // 4 convenience store
        {20, 1, 1, SHADOW},
        // 5 loading area
        {21, 1},
        // 6 chinese new year
        {22, 1, 4, SHADOW, 3},
        // 7 lock pick (cutscene)
        {},
        // 8 level summary screen
        {},
    },
    // level 5
    {},
    // level 6
    {
        // 0 victoria's ward
        {23},
        // 1 orphanage halls
        {24, 1, 5, SHADOW},
        // 2 lenny shoots nun (cutscene)
        {},
        // 3 central heating
        {25, 1, 2, SHADOW, 1},
        // 4 level summary screen
        {},
    },
    // level 7
    {
        // 0 great balls of fire
        {26, 1, 1, SHADOW},
        // 1 level summary screen
        {},
    },
    // level 8
    {
        // 0 gunshop
        {27, 1, 2, VETERAN},
        // 1 level summary screen
        {},
    },
    // level 9
    {
        // 0 streets of hope
        {28, 1, 4, SHADOW, 3},
        // 1 barber shop
        {29, 1, 3, SHADOW, 2},
        // 2 lenny kidnap (cutscene)
        {},
        // 3 level summary screen
        {},
    },
    // level 10
    {
        // 0 the desert
        {30},
        // 1 level summary screen
        {},
    },
    // level 11
    {
        // 0 dog & oil fields (cutscene)
        {},
        // 1 dead end
        {31, 1, 3, SHADOW},
        // 2 old mill
        {32, 1, 1, SHADOW},
        // 3 descent
        {33, 1, 1, SHADOW},
        // 4 factory compound
        {34, 1, 3, SHADOW},
        // 5 level summary screen
        {},
    },
    // level 12
    {
        // 0 test facility
        {35, 1, 5, SHADOW, 1},
        // 1 decontamination
        {36, 1, 2, SHADOW},
        // 2 r&d
        {37, 1, 3, SHADOW, 2},
        // 3 level summary screen
        {},
    },
    // level 13
    {
        // 0 patriot's hangar
        {38, 1, 1, SHADOW},
        // 1 arena
        {39, 1, 2, SHADOW, 1},
        // 2 level summary screen
        {},
    },
    // level 14
    {
        // 0 parking
        {40, 1, 3, SHADOW, 2},
        // 1 reception
        {41, 1, 3, SHADOW, 2},
        // 2 cornfield
        {42, 1, 4, SHADOW, 3},
        // 3 cutscene
        {},
        // 4 level summary screen
        {},
    },
    // level 15
    {},
    // level 16
    {},
    // level 17
    {
        // 0 courthouse
        {43, 1, 1, SHADOW},
        // 1 holding cells
        {44, 1},
        // 2 prison
        {45, 1, 2, SHADOW},
        // 3 electrocution cutscene
        {},
        // 4 level summary screen
        {},
    },
    // level 18
    {
        // 0 county jail
        {46, 1, 2, SHADOW},
        // 1 outgunned
        {47, 1, 1, SHADOW},
        // 2 burn
        {48, 1, 1, SHADOW},
        // 3 hope fair
        {49, 1, 1, SHADOW},
        // 4 outskirts
        {50},
        // 5 church
        {51},
        // 6 level summary screen
        {},
    },
    // level 19
    {},
    // level 20
    {},
    // level 21
    {
        // 0 tailor shop
        {52, 1, 1},
        // 1 level summary screen
        {},
    },
    // level 22
    {
        // 0 blackwater park
        {53, 1, 2, SHADOW},
        // 1 sushi guy cutscene
        {},
        // 2 the penthouse
        {54, 1, 3, SHADOW, 1},
        // 3 level summary screen
        {},
    },
    // level 23
    {},
    // level 24
    {
        // 0 blackwater roof
        {55, 1, 1, SHADOW, 1},
        // 1 level summary screen
        {},
    },
    // level 25
    {
        // 1 burnwood family tomb
        {57, 1, 3, SHADOW, 1},
        // 2 crematorium
        {58, 1, 5, SHADOW, 3},
        // 3 level summary screen
        {},
    },
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
    Stats& stats
) {
    MemoryReader<uint32_t> reader{handle};
    auto tracer
        = mempeep::LogTracer{MempeepOnLogEntry{}, mempeep::LogLevel::ERRORS};
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
    auto& checkpoints = *game.checkpoints_manager.checkpoints;
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
        auto status
            = game_stats[NON_TARGET_CASUALTY] != 0 || game_stats[SPOTTED] != 0
                  ? Status::RED
                  : Status::GREEN;
        stats.rating
            = {map_info.max_rating == AGENT ? "Unrated"
                                            : get_simple_rating_value(status),
               status};
        auto score = get_raw_score(game_stats);
        auto score_shadow = get_best_raw_score(
            game.game_data.level_infos, game.level, checkpoint_index
        );
        if (!score_shadow || score_shadow.value() == 0) {
            logging::error("Unable to read shadow score");
            return false;
        }
        stats.score_for_max_rating
            = (map_info.max_rating * score_shadow.value()) / 100;
        int32_t percent = std::max(0, (100 * score) / score_shadow.value());
        stats.score_total = score;
        stats.score_rating = percent < VETERAN        ? "Agent"
                             : percent < SPECIALIST   ? "Veteran"
                             : percent < PROFESSIONAL ? "Specialist"
                             : percent < SHADOW       ? "Professional"
                                                      : "Shadow";
        // major negative scores
        stats.score_civilian_casualty
            = {STATS_MULTIPLIERS[CIVILIAN_CASUALTY]
                   * game_stats[CIVILIAN_CASUALTY],
               game_stats[CIVILIAN_CASUALTY] ? Status::RED : Status::YELLOW};
        stats.score_non_target_casualty
            = {STATS_MULTIPLIERS[NON_TARGET_CASUALTY]
                   * game_stats[NON_TARGET_CASUALTY],
               game_stats[NON_TARGET_CASUALTY] ? Status::RED : Status::YELLOW};
        stats.score_spotted
            = {STATS_MULTIPLIERS[SPOTTED] * game_stats[SPOTTED],
               game_stats[SPOTTED] ? Status::RED : Status::YELLOW};
        // minor negative or positive scores
        stats.score_pacification
            = {STATS_MULTIPLIERS[PACIFICATION] * game_stats[PACIFICATION],
               game_stats[PACIFICATION] ? Status::RED : Status::YELLOW};
        stats.score_body_hidden
            = {STATS_MULTIPLIERS[BODY_HIDDEN] * game_stats[BODY_HIDDEN],
               game_stats[BODY_HIDDEN] ? Status::GREEN : Status::YELLOW};
        stats.score_headshot
            = {STATS_MULTIPLIERS[HEADSHOT] * game_stats[HEADSHOT],
               game_stats[HEADSHOT] ? Status::GREEN : Status::YELLOW};
        stats.score_silent_kill
            = {STATS_MULTIPLIERS[SILENT_KILL] * game_stats[SILENT_KILL],
               game_stats[SILENT_KILL] ? Status::GREEN : Status::YELLOW};
        // major positive scores
        stats.score_evidence_removed = {
            STATS_MULTIPLIERS[EVIDENCE_REMOVED] * game_stats[EVIDENCE_REMOVED],
            game_stats[EVIDENCE_REMOVED] < map_info.num_evidence
                ? Status::YELLOW
                : Status::GREEN
        };
        stats.score_objective_complete
            = {STATS_MULTIPLIERS[OBJECTIVE_COMPLETE]
                   * game_stats[OBJECTIVE_COMPLETE],
               game_stats[OBJECTIVE_COMPLETE] < map_info.num_objectives
                   ? Status::YELLOW
                   : Status::GREEN};
        // major positive scores for maps with targets
        if (map_info.num_targets > 0) {
            stats.score_signature_kill = {
                STATS_MULTIPLIERS[SIGNATURE_KILL] * game_stats[SIGNATURE_KILL],
                game_stats[SIGNATURE_KILL] < map_info.num_targets
                    ? Status::YELLOW
                    : Status::GREEN
            };
            stats.score_target_kill
                = {STATS_MULTIPLIERS[TARGET_KILL] * game_stats[TARGET_KILL],
                   game_stats[TARGET_KILL] < map_info.num_targets
                       ? Status::YELLOW
                       : Status::GREEN};
            stats.score_silent_assassin_bonus
                = {STATS_MULTIPLIERS[SILENT_ASSASSIN_BONUS]
                       * game_stats[SILENT_ASSASSIN_BONUS],
                   game_stats[SILENT_ASSASSIN_BONUS] == 0 ? Status::YELLOW
                                                          : Status::GREEN};
        } else {
            if (game_stats[SIGNATURE_KILL] != 0)
                logging::warn("no targets but signature kills not zero");
            if (game_stats[TARGET_KILL] != 0)
                logging::warn("no targets but target kills not zero");
            if (game_stats[SILENT_ASSASSIN_BONUS] != 0)
                logging::warn("no targets but silent assassin bonus not zero");
            stats.score_signature_kill = {0};
            stats.score_target_kill = {0};
            stats.score_silent_assassin_bonus = {0};
        }
    }
    return true;
}

constexpr float time_scale = 1.0f / (1024 * 1024);

bool hitman_absolution::update_fast(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    if (stats.map > 0) {
        auto game_time = read<int64_t>(handle, base_ptrs[0] + 0xE24730 + 0x18);
        if (game_time) stats.time = game_time.value() * time_scale;
        return game_time.has_value();
    }
    return true;
}
