#include "stats.hpp"

#include <cstdint>
#include <mempeep/read.hpp>
#include <mempeep/tracers/log_tracer.hpp>
#include <unordered_map>
#include <vector>

#include "../hitman_common/stats.hpp"
#include "../logging.hpp"
#include "../mem/read_write.hpp"
#include "structs.hpp"

struct LevelInfo {
    int map;
    uint32_t level_control_code;
    uint32_t player_code;
};

std::unordered_map<std::string, LevelInfo> level_infos{
    // sanctuary
    {
        "SCENES\\C0-1\\C0-1__MAIN.gms",
        {
            .map = 1,
            .level_control_code = 0x205,
            .player_code = 0x6FD,
        },
    },
    // anathema
    {
        "SCENES\\C1-1\\C1-1__MAIN.gms",
        {
            .map = 2,
            .level_control_code = 0x20E,
            .player_code = 0x657,
        },
    },
    // stakeout
    {
        "SCENES\\C2-1\\C2-1__MAIN.gms",
        {
            .map = 3,
            .level_control_code = 0x2C9,
            .player_code = 0x6D5,
        },
    },
    // kirov
    {
        "SCENES\\C2-2\\C2-2__MAIN.gms",
        {
            .map = 4,
            .level_control_code = 0x228,
            .player_code = 0x62A,
        },
    },
    // tubeway
    {
        "SCENES\\C2-3\\C2-3__MAIN.gms",
        {
            .map = 5,
            .level_control_code = 0x4E,
            .player_code = 0x7DC,
        },
    },
    // invitation
    {
        "SCENES\\C2-4\\C2-4__MAIN.gms",
        {
            .map = 6,
            .level_control_code = 0x2E2,
            .player_code = 0x6F7,
        },
    },
    // tracking
    {
        "SCENES\\C3-1\\C3-1__MAIN.gms",
        {
            .map = 7,
            .level_control_code = 0x2EE,
            .player_code = 0x71D,
        },
    },
    // hidden valley
    {
        "SCENES\\C3-2a\\C3-2a__MAIN.gms",
        {
            .map = 8,
            .level_control_code = 0x2D2,
            .player_code = 0x690,
        },
    },
    // gates
    {
        "SCENES\\C3-2b\\C3-2b__MAIN.gms",
        {
            .map = 9,
            .level_control_code = 0x33A,
            .player_code = 0x790,
        },
    },
    // showdown
    {
        "SCENES\\C3-3\\C3-3__MAIN.gms",
        {
            .map = 10,
            .level_control_code = 0x4DB,
            .player_code = 0x919,
        },
    },
    // basement
    {
        "SCENES\\C4-1\\C4-1__MAIN.gms",
        {
            .map = 11,
            .level_control_code = 0x2B4,
            .player_code = 0x74B,
        },
    },
    // graveyard
    {
        "SCENES\\C4-2\\C4-2__MAIN.gms",
        {
            .map = 12,
            .level_control_code = 0x3D4,
            .player_code = 0x800,
        },
    },
    // jacuzzi
    {
        "SCENES\\C4-3\\C4-3__MAIN.gms",
        {
            .map = 13,
            .level_control_code = 0x235,
            .player_code = 0x638,
        },
    },
    // bazaar
    {
        "SCENES\\C5-1\\C5-1__MAIN.gms",
        {
            .map = 14,
            .level_control_code = 0x27B,
            .player_code = 0x694,
        },
    },
    // motorcade
    {
        "SCENES\\C5-2\\C5-2__MAIN.gms",
        {
            .map = 15,
            .level_control_code = 0x100,
            .player_code = 0x4B5,
        },
    },
    // tunnel rat
    {
        "SCENES\\C5-3\\C5-3__MAIN.gms",
        {
            .map = 16,
            .level_control_code = 0x27B,
            .player_code = 0x63B,
        },
    },
    // temple city
    {
        "SCENES\\C6-1\\C6-1__MAIN.gms",
        {
            .map = 17,
            .level_control_code = 0x191,
            .player_code = 0x5F3,
        },
    },
    // hannelore
    {
        "SCENES\\C6-2\\C6-2__MAIN.gms",
        {
            .map = 18,
            .level_control_code = 0x2C2,
            .player_code = 0x79E,
        },
    },
    // hospitality
    {
        "SCENES\\C6-3\\C6-3__MAIN.gms",
        {
            .map = 19,
            .level_control_code = 0x25B,
            .player_code = 0x80D,
        },
    },
    // revisited
    {
        "SCENES\\C7-1\\C7-1__MAIN.gms",
        {
            .map = 20,
            .level_control_code = 0x2C0,
            .player_code = 0x70A,
        },
    },
    // finale
    {
        "SCENES\\C8-1\\C8-1__MAIN.gms",
        {
            .map = 21,
            .level_control_code = 0x2,
            .player_code = 0x7B0,
        },
    },
};

// https://docs.google.com/spreadsheets/d/1i6dmzcBROqoJlsQjUGY8wxdqwxt2hXzjB9fPVggTf2k/edit?gid=1074822823#gid=1074822823
const std::vector<StatsArray> silent_assassin_combinations
    = {{0, 1, 0, 0, 1, 2, 0, 0}, {0, 1, 0, 0, 0, 5, 0, 0},
       {0, 1, 0, 0, 0, 2, 0, 1}, {0, 0, 0, 1, 2, 0, 0, 0},
       {0, 0, 0, 1, 1, 3, 0, 0}, {0, 0, 0, 1, 1, 0, 0, 1},
       {0, 0, 0, 1, 0, 6, 0, 0}, {0, 0, 0, 1, 0, 3, 0, 1},
       {0, 0, 0, 1, 0, 0, 1, 0}, {0, 0, 0, 1, 0, 0, 0, 2},
       {0, 0, 0, 0, 1, 0, 0, 1}, {1, 1, 1, 0, 0, 2, 0, 0},
       {1, 1, 0, 0, 1, 0, 0, 0}, {1, 1, 0, 0, 0, 3, 0, 0},
       {1, 1, 0, 0, 0, 0, 0, 1}, {1, 0, 1, 1, 1, 0, 0, 0},
       {1, 0, 1, 1, 0, 3, 0, 0}, {1, 0, 1, 1, 0, 0, 0, 1},
       {1, 0, 0, 1, 1, 1, 0, 0}, {1, 0, 0, 1, 0, 4, 0, 0},
       {1, 0, 0, 1, 0, 1, 0, 1}, {1, 0, 0, 0, 1, 1, 0, 0},
       {2, 1, 1, 0, 0, 0, 0, 0}, {2, 1, 0, 0, 0, 1, 0, 0},
       {2, 0, 2, 1, 0, 0, 0, 0}, {2, 0, 1, 1, 0, 1, 0, 0},
       {3, 0, 0, 1, 0, 0, 0, 0}};

// global to avoid allocating large object on stack
static hitman2_silent_assassin::structs::Game game{};

// note: almost same as Hitman Contracts, move to common?
static int32_t measure_aggression(
    const CommonGameStats& stats, int32_t shots_fired
) {
    auto value = 3 * stats.innocents_wounded + 6 * stats.innocents_killed
                 + stats.enemies_wounded + 3 * stats.enemies_killed
                 + 2 * shots_fired + stats.headshots + stats.close_encounters;
    return value;
}

// note: same as Hitman Contracts, move to common?
static int32_t measure_stealth(const CommonGameStats& stats) {
    auto value = stats.alerts + stats.close_encounters;
    return value;
}

bool hitman2_silent_assassin::update_slow(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    const RemoteValue<structs::TGame, uint32_t> remote_game{
        static_cast<uint32_t>(base_ptrs.at(0))
    };
    MemoryReader<uint32_t> reader{handle};
    auto tracer
        = mempeep::LogTracer{MempeepOnLogEntry{}, mempeep::LogLevel::VALUES};
    if (!mempeep::read(remote_game, reader, tracer, game)) return false;
    const auto& scene = game.engine.scene_manager.scene_name.text;
    logging::trace("Scene {}", scene);
    auto iter = level_infos.find(scene);
    if (iter == level_infos.end()) {
        // no map loaded
        stats.map = 0;
        return true;
    }
    const auto& info = iter->second;
    stats.map = info.map;
    stats.map_stage = MapStage::main;  // always render stats
    stats.difficulty = read<int32_t>(handle, label_ptrs.at(250)).value_or(0);
    if (stats.map >= 2) {
        uint32_t level_control_addr{};
        if (!mempeep::read_at(
                game.entity_manager.entities,
                info.level_control_code,
                reader,
                tracer,
                level_control_addr
            ))
            return false;
        const RemoteValue<structs::TLevelControl, uint32_t>
            remote_level_control{level_control_addr};
        structs::LevelControl level_control{};
        if (!mempeep::read(
                remote_level_control, reader, tracer, level_control
            )) {
            logging::warn("Failed to read level control");
            return false;
        }
        uint32_t player_entity_addr{};
        if (!mempeep::read_at(
                game.entity_manager.entities,
                info.player_code,
                reader,
                tracer,
                player_entity_addr
            ))
            return false;
        const RemoteValue<structs::TPlayerEntity, uint32_t>
            remote_player_entity{player_entity_addr};
        structs::PlayerEntity player_entity{};
        if (!mempeep::read(
                remote_player_entity, reader, tracer, player_entity
            )) {
            logging::warn("Failed to read player entity");
            return false;
        }
        const RemoteValue<structs::TPlayer, uint32_t> remote_player{
            game.engine.scene_manager.gref_manager.pool.base
            + (player_entity.gref & 0x3FFFFFFF)
        };
        structs::Player player{};
        if (!mempeep::read(remote_player, reader, tracer, player)) {
            logging::warn("Failed to read player");
            return false;
        }
        CommonGameStats game_stats{0};
        stats.shots_fired.value = player.data.shots_fired;
        game_stats.headshots = level_control.headshots;
        game_stats.enemies_wounded = level_control.enemies_wounded;
        game_stats.enemies_killed = level_control.enemies_killed;
        game_stats.innocents_wounded = level_control.innocents_wounded;
        game_stats.innocents_killed = level_control.innocents_killed;
        game_stats.alerts = level_control.alerts;
        game_stats.close_encounters = level_control.close_encounters;
        process_common_game_stats(
            silent_assassin_combinations, game_stats, stats
        );
        auto stealth = measure_stealth(game_stats);
        stats.stealth
            = {std::lround(1000 * std::pow(0.9, stealth)),
               stealth == 0   ? Status::GREEN
               : stealth == 1 ? Status::YELLOW
                              : Status::RED};
        auto aggression
            = measure_aggression(game_stats, stats.shots_fired.value);
        stats.aggression
            = {std::lround(1000 * std::tanh(0.005 * aggression)),
               aggression <= 5   ? Status::GREEN
               : aggression == 6 ? Status::YELLOW
                                 : Status::RED};
#ifndef NDEBUG
        // validate the two methods (to be removed when confirmed stable)
        bool is_sa_1 = (stats.rating.status == Status::GREEN);
        bool is_sa_2 = (stealth <= 1 && aggression <= 6);
        if (is_sa_1 != is_sa_2) {
            logging::error(
                "silent assassin status stealth/aggression mismatch"
            );
        }
        // TODO validate the values themselves when on the stats screen
#endif
    } else {
        stats.rating = {"Unrated", Status::GREEN};
    }
    return true;
}

bool hitman2_silent_assassin::update_fast(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    if (stats.map > 0) {
        const auto& base_ptr = base_ptrs.at(0);
        auto time = read<int32_t>(
            handle,
            base_ptr + 0x2A6C58,
            {0x118, 0xB38, 0x8, 0x1084, 0x24},
            INT32_MAX
        );
        if (time) stats.time = time.value() * 0.0166666666666666f;  // 1 / 60.0f
        return time.has_value();
    }
    return true;
}