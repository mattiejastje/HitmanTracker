#include "stats.hpp"

#include <spdlog/spdlog.h>

#include <cstdint>
#include <mempeep/read.hpp>
#include <mempeep/tracers/log_tracer.hpp>
#include <unordered_map>
#include <vector>

#include "../hitman_common/read_lethed.hpp"
#include "../hitman_common/stats.hpp"
#include "../mem/read_write.hpp"
#include "structs.hpp"

using namespace hitman_common;

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

// note: almost same as Hitman Contracts, move to common?
static int32_t measure_aggression(const StatsArray<int32_t>& stats) {
    auto value = 3 * stats[INNOCENTS_WOUNDED] + 6 * stats[INNOCENTS_KILLED]
                 + stats[ENEMIES_WOUNDED] + 3 * stats[ENEMIES_KILLED]
                 + 2 * stats[SHOTS_FIRED] + stats[HEADSHOTS]
                 + stats[CLOSE_ENCOUNTERS];
    return value;
}

// note: same as Hitman Contracts, move to common?
static int32_t measure_stealth(const StatsArray<int32_t>& stats) {
    auto value = stats[ALERTS] + stats[CLOSE_ENCOUNTERS];
    return value;
}

GameStatsSlow hitman2_silent_assassin::update_slow(Version version) {
    return [](const std::filesystem::path& exe_path,
              void* handle,
              const BasePtrs& base_ptrs,
              const LabelPtrs& label_ptrs,
              std::any& remote_state_any,
              std::any& stats_any) {
        auto& game = std::any_cast<structs::Game&>(remote_state_any);
        auto& stats = std::any_cast<Stats&>(stats_any);
        const RemoteValue<structs::TGame, uint32_t> remote_game{
            static_cast<uint32_t>(base_ptrs.at(0))
        };
        MemoryReader<uint32_t> reader{handle};
        auto tracer = mempeep::LogTracer{
            MempeepOnLogEntry{}, mempeep::LogLevel::ERRORS
        };
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
        stats.difficulty = read_lethed(
                               game.property_manager.data,
                               game.property_manager.data_used,
                               reader,
                               tracer
        )
                               .value_or(0);
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
                    const RemoteValue<
                        Primitive<structs::LevelControl>,
                        uint32_t>
                        remote_level_control{level_control_addr};
                    if (!mempeep::read(
                            remote_level_control, reader, tracer, level_control
                        )) {
                        spdlog::warn("Failed to read level control");
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
                    spdlog::warn("Failed to read player");
                    return false;
                }
                if (player.data.player_gref
                    != (0x40000000 | info.player_gref)) {
                    spdlog::warn("Player gref validation check failed");
                    return false;
                }
            }
            StatsArray<int32_t> game_stats{};
            game_stats[SHOTS_FIRED] = player.data.shots_fired;
            game_stats[HEADSHOTS] = level_control.headshots;
            game_stats[ENEMIES_WOUNDED] = level_control.enemies_wounded;
            game_stats[ENEMIES_KILLED] = level_control.enemies_killed;
            game_stats[INNOCENTS_WOUNDED] = level_control.innocents_wounded;
            game_stats[INNOCENTS_KILLED] = level_control.innocents_killed;
            game_stats[ALERTS] = level_control.alerts;
            game_stats[CLOSE_ENCOUNTERS] = level_control.close_encounters;
            process_game_stats(
                measure_aggression, measure_stealth, game_stats, stats
            );
        } else {
            stats.rating = {"Unrated", Status::GREEN};
        }
        return true;
    };
}

GameStatsFast hitman2_silent_assassin::update_fast(Version version) {
    return [](void* handle,
              const BasePtrs& base_ptrs,
              const LabelPtrs& label_ptrs,
              std::any& stats_any) {
        auto& stats = std::any_cast<Stats&>(stats_any);
        if (stats.map > 0) {
            const auto& base_ptr = base_ptrs.at(0);
            auto time = read<int32_t>(
                handle,
                base_ptr + 0x2A6C58,
                {0x118, 0xB38, 0x8, 0x1084, 0x24},
                INT32_MAX
            );
            if (time)
                stats.time = time.value() * 0.0166666666666666f;  // 1 / 60.0f
            return time.has_value();
        }
        return true;
    };
}