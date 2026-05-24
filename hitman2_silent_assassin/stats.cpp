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
    std::size_t level_control_code;
    uint32_t player_gref;
};

std::unordered_map<std::string, LevelInfo> level_infos{
    // sanctuary
    {
        "SCENES\\C0-1\\C0-1__MAIN.gms",
        {
            .map = 1,
            .level_control_code = 0x205,
            .player_gref = 0x38EE0,
        },
    },
    // anathema
    {
        "SCENES\\C1-1\\C1-1__MAIN.gms",
        {
            .map = 2,
            .level_control_code = 0x20E,
            .player_gref = 0x68F20,
        },
    },
    // stakeout
    {
        "SCENES\\C2-1\\C2-1__MAIN.gms",
        {
            .map = 3,
            .level_control_code = 0x2C9,
            .player_gref = 0x9FE10,
        },
    },
    // kirov
    {
        "SCENES\\C2-2\\C2-2__MAIN.gms",
        {
            .map = 4,
            .level_control_code = 0x228,
            .player_gref = 0x54A10,
        },
    },
    // tubeway
    {
        "SCENES\\C2-3\\C2-3__MAIN.gms",
        {
            .map = 5,
            .level_control_code = 0x4E,
            .player_gref = 0x1E610,
        },
    },
    // invitation
    {
        "SCENES\\C2-4\\C2-4__MAIN.gms",
        {
            .map = 6,
            .level_control_code = 0x2E2,
            .player_gref = 0x108CC0,
        },
    },
    // tracking
    {
        "SCENES\\C3-1\\C3-1__MAIN.gms",
        {
            .map = 7,
            .level_control_code = 0x2EE,
            .player_gref = 0x55650,
        },
    },
    // hidden valley
    {
        "SCENES\\C3-2a\\C3-2a__MAIN.gms",
        {
            .map = 8,
            .level_control_code = 0x2D2,
            .player_gref = 0x5F670,
        },
    },
    // gates
    {
        "SCENES\\C3-2b\\C3-2b__MAIN.gms",
        {
            .map = 9,
            .level_control_code = 0x33A,
            .player_gref = 0x4EDC0,
        },
    },
    // showdown
    {
        "SCENES\\C3-3\\C3-3__MAIN.gms",
        {
            .map = 10,
            .level_control_code = 0x4DB,
            .player_gref = 0x62A10,
        },
    },
    // basement
    {
        "SCENES\\C4-1\\C4-1__MAIN.gms",
        {
            .map = 11,
            .level_control_code = 0x2B4,
            .player_gref = 0x77620,
        },
    },
    // graveyard
    {
        "SCENES\\C4-2\\C4-2__MAIN.gms",
        {
            .map = 12,
            .level_control_code = 0x3D4,
            .player_gref = 0x811E0,
        },
    },
    // jacuzzi
    {
        "SCENES\\C4-3\\C4-3__MAIN.gms",
        {
            .map = 13,
            .level_control_code = 0x235,
            .player_gref = 0x44630,
        },
    },
    // bazaar
    {
        "SCENES\\C5-1\\C5-1__MAIN.gms",
        {
            .map = 14,
            .level_control_code = 0x27B,
            .player_gref = 0x3CFA0,
        },
    },
    // motorcade
    {
        "SCENES\\C5-2\\C5-2__MAIN.gms",
        {
            .map = 15,
            .level_control_code = 0x100,
            .player_gref = 0x35590,
        },
    },
    // tunnel rat
    {
        "SCENES\\C5-3\\C5-3__MAIN.gms",
        {
            .map = 16,
            .level_control_code = 0x27B,
            .player_gref = 0x4D310,
        },
    },
    // temple city
    {
        "SCENES\\C6-1\\C6-1__MAIN.gms",
        {
            .map = 17,
            .level_control_code = 0x191,
            .player_gref = 0x6F820,
        },
    },
    // hannelore
    {
        "SCENES\\C6-2\\C6-2__MAIN.gms",
        {
            .map = 18,
            .level_control_code = 0x2C2,
            .player_gref = 0x60860,
        },
    },
    // hospitality
    {
        "SCENES\\C6-3\\C6-3__MAIN.gms",
        {
            .map = 19,
            .level_control_code = 0x25B,
            .player_gref = 0xC5110,
        },
    },
    // revisited
    {
        "SCENES\\C7-1\\C7-1__MAIN.gms",
        {
            .map = 20,
            .level_control_code = 0x2C0,
            .player_gref = 0xA0270,
        },
    },
    // finale
    {
        "SCENES\\C8-1\\C8-1__MAIN.gms",
        {
            .map = 21,
            .level_control_code = 0x2,
            .player_gref = 0x15B60,
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

template <IsTracer Tracer>
static std::optional<uint32_t> read_lethed(
    const hitman2_silent_assassin::structs::PropertyManager& property_manager,
    MemoryReader<uint32_t> reader,
    Tracer tracer
) {
    uint32_t offset = 0;
    uint32_t index = 0;  // to avoid infinite loop
    while (offset < property_manager.data_used || index < 0x200) {
        mempeep::RemoteValue<
            hitman2_silent_assassin::structs::TPropertyManagerRecord,
            uint32_t>
            remote_record{property_manager.data + offset};
        hitman2_silent_assassin::structs::PropertyManagerRecord record{};
        if (!mempeep::read(remote_record, reader, tracer, record)) {
            logging::warn("Unable to read property manager record");
            return {};
        }
        // "lethed" property has record size 0x1C so filter on that first
        if (record.is_active && record.record_size == 0x1C) {
            hitman2_silent_assassin::structs::Property property{};
            if (!mempeep::read(record.property, reader, tracer, property)) {
                logging::warn("Unable to read property");
                return {};
            }
            if (property.key == "lethed") {
                if (property.size != 4) {
                    logging::warn("Property \"lethed\" has wrong size");
                    return {};
                }
                int32_t lethed;
                if (!reader(property_manager.data + offset + 0x18, 4, &lethed)) {
                    logging::warn("Unable to read property \"lethed\" value");
                    return {};
                }
                return lethed;
            }
        }
        offset += record.record_size;
    }
    if (index == 0x200) logging::warn("Too many properties");
    return {};
}

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
        = mempeep::LogTracer{MempeepOnLogEntry{}, mempeep::LogLevel::ERRORS};
    if (!mempeep::read(remote_game, reader, tracer, game)) return false;
    const auto& scene = game.engine.scene_manager.scene_name.text;
    auto iter = level_infos.find(scene);
    if (iter == level_infos.end()) {
        // no map loaded
        stats.map = 0;
        return true;
    }
    const auto& info = iter->second;
    stats.map = info.map;
    stats.map_stage = MapStage::main;  // always render stats
    stats.difficulty
        = read_lethed(game.property_manager, reader, tracer).value_or(0);
    if (stats.map >= 2) {
        structs::LevelControl level_control{};
        // entities can be briefly null if mission is still loading
        if (game.entity_manager.entities) {
            uint32_t level_control_addr{};
            if (!mempeep::read_at(
                    *game.entity_manager.entities,
                    info.level_control_code,
                    reader,
                    tracer,
                    level_control_addr
                ))
                return false;
            // address can be briefly zero if mission is still loading
            if (level_control_addr != 0) {
                const RemoteValue<Primitive<structs::LevelControl>, uint32_t>
                    remote_level_control{level_control_addr};
                if (!mempeep::read(
                        remote_level_control, reader, tracer, level_control
                    )) {
                    logging::warn("Failed to read level control");
                    return false;
                }
            }
        }
        structs::Player player{};
        // gref_manager can be null if mission is still loading
        if (game.engine.scene_manager.gref_manager) {
            const RemoteValue<structs::TPlayer, uint32_t> remote_player{
                game.engine.scene_manager.gref_manager->pool.base
                + info.player_gref
            };
            if (!mempeep::read(remote_player, reader, tracer, player)) {
                logging::warn("Failed to read player");
                return false;
            }
            if (player.data.player_gref != (0x40000000 | info.player_gref)) {
                logging::warn("Player gref validation check failed");
                return false;
            }
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