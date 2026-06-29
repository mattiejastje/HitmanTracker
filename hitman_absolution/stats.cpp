#include "stats.hpp"

#include <format>
#include <iostream>  // std::cout
#include <mempeep/read.hpp>
#include <mempeep/tracers/log_tracer.hpp>

#include "../hitman_common/simple_rating.hpp"
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
    // 40000 is default value for unrated maps
    int shadow_raw_score_threshold = 40000;
    int num_targets;
};

// level, checkpoint -> map info
const std::vector<std::vector<MapInfo>> scenes = {
    // level 0
    {
        // 0 garden
        {.map = 1},
        // 1 greenhouse
        {.map = 2},
        // 2 cliffside
        {
            .map = 3,
            .num_evidence = 1,
        },
        // 3 mansion ground
        {
            .map = 4,
            .num_evidence = 1,
            .num_objectives = 2,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 11000,
        },
        // 4 mansion 2nd
        {.map = 5},
        // 5 level summary screen
        {},
    },
    // level 1
    {
        // 0 king of chinatown
        {
            .map = 6,
            .num_evidence = 1,
            .num_objectives = 3,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 31000,
            .num_targets = 1,
        },
        // 1 level summary screen
        {},
    },
    // level 2
    {
        // 0 terminus hotel
        {
            .map = 7,
            .num_evidence = 1,
            .num_objectives = 1,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 6000,
        },
        // 1 elevator going up (cutscene)
        {},
        // 2 upper floors
        {
            .map = 8,
            .num_evidence = 1,
            .num_objectives = 2,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 11000,
        },
        // 3 room 899
        {.map = 9},
        // 4 level summary screen
        {},
    },
    // level 3
    {
        // 0 burning hotel
        {
            .map = 10,
            .num_evidence = 0,
            .num_objectives = 1,
            .max_rating = SPECIALIST,
            .shadow_raw_score_threshold = 6000,
        },
        // 1 library
        {
            .map = 11,
            .num_evidence = 1,
            .num_objectives = 1,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 6000,
        },
        // 2 pigeon coop
        {.map = 12},
        // 3 rooftops
        {.map = 13},
        // 4 shangri-la
        {
            .map = 14,
            .num_evidence = 1,
            .num_objectives = 1,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 6000,
        },
        // 5 elevator going down (cutscene)
        {},
        // 6 train station
        {
            .map = 15,
            .num_evidence = 1,
            .num_objectives = 3,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 16000,
        },
        // 7 level summary screen
        {},
    },
    // level 4
    {
        // 0 courtyard
        {
            .map = 16,
            .num_evidence = 1,
            .num_objectives = 1,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 6000,
        },
        // 1 vixen club
        {
            .map = 17,
            .num_evidence = 1,
            .num_objectives = 3,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 31000,
            .num_targets = 1,
        },
        // 2 dressing rooms
        {
            .map = 18,
            .num_evidence = 0,
            .num_objectives = 1,
            .max_rating = SPECIALIST,
            .shadow_raw_score_threshold = 6000,
        },
        // 3 derelict building
        {
            .map = 19,
            .num_evidence = 1,
            .num_objectives = 1,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 6000,
        },
        // 4 convenience store
        {
            .map = 20,
            .num_evidence = 1,
            .num_objectives = 1,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 6000,
        },
        // 5 loading area
        {
            .map = 21,
            .num_evidence = 1,
        },
        // 6 chinese new year
        {
            .map = 22,
            .num_evidence = 1,
            .num_objectives = 4,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 21000,
            .num_targets = 3,
        },
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
        {.map = 23},
        // 1 orphanage halls
        {
            .map = 24,
            .num_evidence = 1,
            .num_objectives = 5,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 26000,
        },
        // 2 lenny shoots nun (cutscene)
        {},
        // 3 central heating
        {
            .map = 25,
            .num_evidence = 1,
            .num_objectives = 2,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 26000,
            .num_targets = 1,
        },
        // 4 level summary screen
        {},
    },
    // level 7
    {
        // 0 great balls of fire
        {
            .map = 26,
            .num_evidence = 1,
            .num_objectives = 1,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 6000,
        },
        // 1 level summary screen
        {},
    },
    // level 8
    {
        // 0 gunshop
        {
            .map = 27,
            .num_evidence = 1,
            .num_objectives = 2,
            .max_rating = VETERAN,
            .shadow_raw_score_threshold = 16000,
        },
        // 1 level summary screen
        {},
    },
    // level 9
    {
        // 0 streets of hope
        {
            .map = 28,
            .num_evidence = 1,
            .num_objectives = 4,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 66000,
            .num_targets = 3,
        },
        // 1 barber shop
        {
            .map = 29,
            .num_evidence = 1,
            .num_objectives = 3,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 51000,
            .num_targets = 2,
        },
        // 2 lenny kidnap (cutscene)
        {},
        // 3 level summary screen
        {},
    },
    // level 10
    {
        // 0 the desert
        {.map = 30},
        // 1 level summary screen
        {},
    },
    // level 11
    {
        // 0 dog & oil fields (cutscene)
        {},
        // 1 dead end
        {
            .map = 31,
            .num_evidence = 1,
            .num_objectives = 3,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 16000,
        },
        // 2 old mill
        {
            .map = 32,
            .num_evidence = 1,
            .num_objectives = 1,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 6000,
        },
        // 3 descent
        {
            .map = 33,
            .num_evidence = 1,
            .num_objectives = 2,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 11000,
        },
        // 4 factory compound
        {
            .map = 34,
            .num_evidence = 1,
            .num_objectives = 3,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 16000,
        },
        // 5 level summary screen
        {},
    },
    // level 12
    {
        // 0 test facility
        {
            .map = 35,
            .num_evidence = 1,
            .num_objectives = 5,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 41000,
            .num_targets = 1,
        },
        // 1 decontamination
        {
            .map = 36,
            .num_evidence = 1,
            .num_objectives = 2,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 11000,
        },
        // 2 r&d
        {
            .map = 37,
            .num_evidence = 1,
            .num_objectives = 3,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 31000,
            .num_targets = 2,
        },
        // 3 level summary screen
        {},
    },
    // level 13
    {
        // 0 patriot's hangar
        {
            .map = 38,
            .num_evidence = 1,
            .num_objectives = 1,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 6000,
        },
        // 1 arena
        {
            .map = 39,
            .num_evidence = 1,
            .num_objectives = 2,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 26000,
            .num_targets = 1,
        },
        // 2 level summary screen
        {},
    },
    // level 14
    {
        // 0 parking
        {
            .map = 40,
            .num_evidence = 1,
            .num_objectives = 3,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 31000,
            .num_targets = 2,
        },
        // 1 reception
        {
            .map = 41,
            .num_evidence = 1,
            .num_objectives = 3,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 31000,
            .num_targets = 2,
        },
        // 2 cornfield
        {
            .map = 42,
            .num_evidence = 1,
            .num_objectives = 4,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 66000,
            .num_targets = 3,
        },
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
        {
            .map = 43,
            .num_evidence = 1,
            .num_objectives = 1,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 6000,
        },
        // 1 holding cells
        {
            .map = 44,
            .num_evidence = 1,
            // not 40000 for unknown reasons
            .shadow_raw_score_threshold = 20000,
        },
        // 2 prison
        {
            .map = 45,
            .num_evidence = 1,
            .num_objectives = 2,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 11000,
        },
        // 3 electrocution cutscene
        {},
        // 4 level summary screen
        {},
    },
    // level 18
    {
        // 0 county jail
        {
            .map = 46,
            .num_evidence = 1,
            .num_objectives = 2,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 11000,
        },
        // 1 outgunned
        {
            .map = 47,
            .num_evidence = 1,
            .num_objectives = 1,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 6000,
        },
        // 2 burn
        {
            .map = 48,
            .num_evidence = 1,
            .num_objectives = 1,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 6000,
        },
        // 3 hope fair
        {
            .map = 49,
            .num_evidence = 1,
            .num_objectives = 1,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 6000,
        },
        // 4 outskirts
        {.map = 50},
        // 5 church
        {.map = 51},
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
        {
            .map = 52,
            .num_evidence = 1,
            .num_objectives = 1,
        },
        // 1 level summary screen
        {},
    },
    // level 22
    {
        // 0 blackwater park
        {
            .map = 53,
            .num_evidence = 1,
            .num_objectives = 2,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 11000,
        },
        // 1 sushi guy cutscene
        {},
        // 2 the penthouse
        {
            .map = 54,
            .num_evidence = 1,
            .num_objectives = 3,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 31000,
            .num_targets = 1,
        },
        // 3 level summary screen
        {},
    },
    // level 23
    {},
    // level 24
    {
        // 0 blackwater roof
        {
            .map = 55,
            .num_evidence = 1,
            .num_objectives = 1,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 21000,
            .num_targets = 1,
        },
        // 1 level summary screen
        {},
    },
    // level 25
    {
        // 0 cemetary entrance
        {
            .map = 56,
            .num_evidence = 1,
            .num_objectives = 1,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 6000,
        },
        // 1 burnwood family tomb
        {
            .map = 57,
            .num_evidence = 1,
            .num_objectives = 3,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 31000,
            .num_targets = 1,
        },
        // 2 crematorium
        {
            .map = 58,
            .num_evidence = 1,
            .num_objectives = 5,
            .max_rating = SHADOW,
            .shadow_raw_score_threshold = 76000,
            .num_targets = 3,
        },
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

static int32_t get_raw_score(const std::array<int16_t, 0x64>& values) {
    int32_t score = 0;
    for (std::size_t index = 1; index < 13; index++) {
        score += values[index] * STATS_MULTIPLIERS[index];
    }
    return score;
};

static const hitman_absolution::structs::GameDataLevelInfo* get_level_info(
    const std::vector<hitman_absolution::structs::GameDataLevelInfo>&
        level_infos,
    int32_t level
) {
    for (const auto& level_info : level_infos)
        if (level_info.level_data.level == level) return &level_info;
    return nullptr;
}

static std::optional<int32_t> get_shadow_raw_score_threshold(
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
                if (data) return data->shadow_raw_score_threshold;
            }
        }
    }
    return {};
}

template <typename T, IsMemoryReader Reader, IsTracer Tracer>
bool read_game(
    uint32_t address,
    Reader& reader,
    Tracer& tracer,
    hitman_absolution::structs::Game& game
) {
    return mempeep::read(
        RemoteValue<T, uint32_t>{address}, reader, tracer, game
    );
}

// global to avoid allocating large object on stack
static hitman_absolution::structs::Game game{};
static int64_t start_time = 0;

GameStatsSlow hitman_absolution::update_slow(
    const settings::HMA& hma, GameVersion game_version
) {
    return [&hma, game_version](
               const std::filesystem::path& exe_path,
               void* handle,
               const BasePtrs& base_ptrs,
               const LabelPtrs& label_ptrs,
               Stats& stats
           ) {
        MemoryReader<uint32_t> reader{handle};
        auto tracer = mempeep::LogTracer{
            MempeepOnLogEntry{}, mempeep::LogLevel::ERRORS
        };
        const auto address = static_cast<uint32_t>(base_ptrs.at(0));
        switch (game_version) {
            case GameVersion::Steam:
                if (!read_game<structs::TGameSteam>(
                        address, reader, tracer, game
                    ))
                    return false;
                break;
            case GameVersion::GOG:
                if (!read_game<structs::TGameGOG>(
                        address, reader, tracer, game
                    ))
                    return false;
                break;
            default:
                return false;
        }
        stats.difficulty = game.global_data.difficulty;
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
        logging::trace(
            "Level {} at checkpoint {}", game.level, checkpoint_index
        );
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
        if (stats.map != map_info.map) {
            // map changed: reset "spotted" and start_time
            write<int32_t>(handle, label_ptrs.at(150), 0);
            start_time = game.time_manager.game_time;
        }
        stats.map = map_info.map;
        stats.map_stage = MapStage::main;  // always render stats
        if (stats.map > 0) {
            auto& stats_manager = game.stats_manager;
            auto& game_stats
                = stats_manager.values[game.level][checkpoint_index];
            if (map_info.max_rating == AGENT) {
                // stats are always 0 for unrated maps so try and fix stats here
                // event_manager.kills_per_npc_type?
                // npc type 1 = civ, 2 = guard/cop, 3 = target: easy to count
                // but it does not reset between checkpoints
                // event_manager.npcs_killed?
                // resets between checkpoints
                // but includes targets
                // however unrated maps never have targets! so we can use that
                game_stats[NON_TARGET_CASUALTY]
                    = game.event_manager.npcs_killed;
                // event_type_2 0x23 = spotted
                // event_manager.events_per_event_type_2[0x23] gets stuck once
                // set... so we use a code hook
                auto spotted = read<int32_t>(handle, label_ptrs.at(150));
                if (!spotted) logging::warn("Unable to read spotted value");
                game_stats[SPOTTED] = spotted.value_or(0);
            }
            if (hma.always_track_sa || map_info.num_targets > 0) {
                auto status = game_stats[NON_TARGET_CASUALTY] != 0
                                      || game_stats[SPOTTED] != 0
                                  ? Status::RED
                                  : Status::GREEN;
                stats.rating = {get_simple_rating_value(status), status};
            } else {
                stats.rating = {"No Targets", Status::GREEN};
            }
            auto score = get_raw_score(game_stats);
#ifndef NDEBUG
            auto maybe_shadow_raw_score_threshold
                = get_shadow_raw_score_threshold(
                    game.game_data.level_infos, game.level, checkpoint_index
                );
            if (!maybe_shadow_raw_score_threshold) {
                logging::error("Unable to read shadow score");
                return false;
            }
            if (maybe_shadow_raw_score_threshold.value()
                != map_info.shadow_raw_score_threshold) {
                logging::error(
                    "Wrong shadow score for level {} checkpoint {}: {} -> {}",
                    game.level,
                    checkpoint_index,
                    map_info.shadow_raw_score_threshold,
                    maybe_shadow_raw_score_threshold.value()
                );
            }
#endif
            auto shadow_raw_score_threshold
                = map_info.shadow_raw_score_threshold;
            stats.score_for_max_rating
                = (map_info.max_rating * shadow_raw_score_threshold) / 100;
            int32_t percent
                = std::max(0, (100 * score) / shadow_raw_score_threshold);
            stats.score_total = score;
            stats.score_rating = map_info.max_rating == AGENT ? "Unrated"
                                 : percent < VETERAN          ? "Agent"
                                 : percent < SPECIALIST       ? "Veteran"
                                 : percent < PROFESSIONAL     ? "Specialist"
                                 : percent < SHADOW           ? "Professional"
                                                              : "Shadow";
            // major negative scores
            stats.score_civilian_casualty
                = {STATS_MULTIPLIERS[CIVILIAN_CASUALTY]
                       * game_stats[CIVILIAN_CASUALTY],
                   game_stats[CIVILIAN_CASUALTY] ? Status::RED
                                                 : Status::YELLOW};
            stats.score_non_target_casualty
                = {STATS_MULTIPLIERS[NON_TARGET_CASUALTY]
                       * game_stats[NON_TARGET_CASUALTY],
                   game_stats[NON_TARGET_CASUALTY] ? Status::RED
                                                   : Status::YELLOW};
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
            if (map_info.num_evidence > 0) {
                stats.score_evidence_removed
                    = {STATS_MULTIPLIERS[EVIDENCE_REMOVED]
                           * game_stats[EVIDENCE_REMOVED],
                       game_stats[EVIDENCE_REMOVED] < map_info.num_evidence
                           ? Status::YELLOW
                           : Status::GREEN};
            } else {
                if (game_stats[EVIDENCE_REMOVED] != 0)
                    logging::warn("no evidence but evidence removed not zero");
                stats.score_evidence_removed = {0};
            }
            stats.score_objective_complete
                = {STATS_MULTIPLIERS[OBJECTIVE_COMPLETE]
                       * game_stats[OBJECTIVE_COMPLETE],
                   game_stats[OBJECTIVE_COMPLETE] < map_info.num_objectives
                       ? Status::YELLOW
                       : Status::GREEN};
            // major positive scores for maps with targets
            if (map_info.num_targets > 0) {
                stats.score_signature_kill
                    = {STATS_MULTIPLIERS[SIGNATURE_KILL]
                           * game_stats[SIGNATURE_KILL],
                       game_stats[SIGNATURE_KILL] < map_info.num_targets
                           ? Status::YELLOW
                           : Status::GREEN};
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
                    logging::warn(
                        "no targets but silent assassin bonus not zero"
                    );
                stats.score_signature_kill = {0};
                stats.score_target_kill = {0};
                stats.score_silent_assassin_bonus = {0};
            }
        }
        return true;
    };
}

constexpr float time_scale = 1.0f / (1024 * 1024);

bool hitman_absolution::update_fast(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    if (stats.map > 0) {
        const auto& base_ptr = base_ptrs.at(0);
        auto game_time = read<int64_t>(handle, base_ptr + 0xE24730 + 0x18);
        if (game_time) stats.time = (*game_time - start_time) * time_scale;
        return game_time.has_value();
    }
    return true;
}
