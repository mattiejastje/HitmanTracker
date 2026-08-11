#include "stats.hpp"

#include <spdlog/spdlog.h>

#include <cmath>
#include <mempeep/read.hpp>
#include <mempeep/tracers/log_tracer.hpp>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../hitman_common/read_lethed.hpp"
#include "../hitman_common/stats.hpp"
#include "../mem/read_write.hpp"
#include "structs.hpp"

using namespace hitman_common;

// unordered_map for fast lookup
const std::unordered_map<std::string, int> scenes = {
    {R"(SCENES\C01-1\C01-1_MAIN.gms)", 1},
    {R"(SCENES\C01-2\C01-2_MAIN.gms)", 2},
    {R"(SCENES\C02-1\C02-1_MAIN.gms)", 3},
    {R"(SCENES\C03-1\C03-1_MAIN.gms)", 4},
    {R"(SCENES\C06-1\C06-1_MAIN.gms)", 5},
    {R"(SCENES\C06-2\C06-2_MAIN.gms)", 6},
    {R"(SCENES\C07-1\C07-1_MAIN.gms)", 7},
    {R"(SCENES\C08-1\C08-1_MAIN.gms)", 8},
    {R"(SCENES\C08-2\C08-2_MAIN.gms)", 9},
    {R"(SCENES\C08-3\C08-3_MAIN.gms)", 10},
    {R"(SCENES\C08-4\C08-4_MAIN.gms)", 11},
    {R"(SCENES\C09-1\C09-1_MAIN.gms)", 12},
};

static int32_t _measure_aggression(
    const StatsArray<int32_t>& stats, bool is_first_map
) {
    auto value = 3 * stats[INNOCENTS_WOUNDED] + 6 * stats[INNOCENTS_KILLED]
                 + stats[ENEMIES_WOUNDED] + 3 * stats[ENEMIES_KILLED]
                 + 2 * stats[SHOTS_FIRED] + stats[HEADSHOTS]
                 + stats[CLOSE_ENCOUNTERS];
    // game uses 100 * std::tanh(0.005 * value) and truncates it to zero
    // 0 -> 0.0
    // 1 -> 0.4999958333749996
    // 2 -> 0.999966667999946
    // 3 -> 1.499887510124078
    // 4 -> 1.9997333759930933
    // 5 -> 2.4994792968420687
    // 6 -> 2.9991003238820144
    // 7 -> 3.4985715332779446
    // here we use the raw value (maximum clarity)
    if (value <= 6) {
        // cap min at 7 (aggression = 3) if innocents hurt or close encounter on
        // 1st map
        if (stats[INNOCENTS_KILLED] > 0 || stats[INNOCENTS_WOUNDED] > 0)
            return 7;
        else if (is_first_map && stats[CLOSE_ENCOUNTERS] > 0)
            return 7;
    } else if (stats[CLOSE_ENCOUNTERS] == 0 && stats[ENEMIES_KILLED] == 0
               && stats[ENEMIES_WOUNDED] == 0 && stats[INNOCENTS_KILLED] == 0
               && stats[INNOCENTS_WOUNDED] == 0 && stats[HEADSHOTS] == 0) {
        // cap max at 6 (aggression = 2) in distraction shots only scenario
        return 6;
    }
    return value;
}

// measure aggression on map 1
static int32_t measure_aggression_1(const StatsArray<int32_t>& stats) {
    return _measure_aggression(stats, true);
}

// measure aggression on other maps
static int32_t measure_aggression_2(const StatsArray<int32_t>& stats) {
    return _measure_aggression(stats, false);
}

static int32_t measure_stealth(const StatsArray<int32_t>& stats) {
    auto value = stats[ALERTS] + stats[CLOSE_ENCOUNTERS];
    return value;
}

GameStatsSlow hitman_contracts::update_slow(Version version) {
    return [version](
               const std::filesystem::path& exe_path,
               void* handle,
               const BasePtrs& base_ptrs,
               const LabelPtrs& label_ptrs,
               std::any& remote_state_any,
               std::any& stats_any
           ) {
        auto& game = std::any_cast<structs::HitmanContracts&>(remote_state_any);
        auto& stats = std::any_cast<Stats&>(stats_any);
        MemoryReader<uint32_t> reader{handle};
        auto tracer = mempeep::LogTracer{
            MempeepOnLogEntry{}, mempeep::LogLevel::ERRORS
        };
        const auto address = static_cast<uint32_t>(base_ptrs.at(0));
        switch (version) {
            case Version::Steam:
                if (!read_at_address<structs::THitmanContractsSteam>(
                        address, reader, tracer, game
                    ))
                    return false;
                break;
            case Version::GOG:
                if (!read_at_address<structs::THitmanContractsGOG>(
                        address, reader, tracer, game
                    ))
                    return false;
                break;
            default:
                return false;
        }
        const auto& scene = game.engine.scene_manager.scene_name.text;
        spdlog::trace("Scene {}", scene);
        auto iter = scenes.find(scene);
        if (iter != scenes.end()) {
            stats.map = iter->second;
            stats.map_stage = MapStage::main;
            spdlog::trace("Map {}", stats.map);
        } else {
            spdlog::trace("Unhandled scene {}", scene);
            stats.map = 0;
        }
        stats.difficulty = read_property_int32(
                               game.property_manager.data,
                               game.property_manager.data_used,
                               "lethed",
                               reader,
                               tracer
        )
                               .value_or(0);
        if (stats.map >= 1) {
            const auto& player_data = game.player.data;
            const auto& player_stats = game.player.stats;
            if (player_stats) {
                StatsArray<int32_t> game_stats{};
                game_stats[SHOTS_FIRED]
                    = player_data ? player_data->shots_fired : 0;
                game_stats[HEADSHOTS] = player_stats->headshots;
                game_stats[ENEMIES_WOUNDED] = player_stats->enemies_wounded;
                game_stats[ENEMIES_KILLED] = player_stats->enemies_killed;
                game_stats[INNOCENTS_WOUNDED] = player_stats->innocents_wounded;
                game_stats[INNOCENTS_KILLED] = player_stats->innocents_killed;
                game_stats[ALERTS] = player_stats->alerts;
                game_stats[CLOSE_ENCOUNTERS] = player_stats->close_encounters;
                StatsFunc measure_aggression = stats.map == 1
                                                   ? measure_aggression_1
                                                   : measure_aggression_2;
                process_game_stats(
                    measure_aggression, measure_stealth, game_stats, stats
                );
            };
        }
        return true;
    };
}

constexpr float seconds_per_tick = 1.0f / 1024;

GameStatsFast hitman_contracts::update_fast(Version version) {
    const uint32_t engine_offset
        = version == Version::Steam ? 0x39457C : 0x393DDC;
    return [engine_offset](
               void* handle,
               const BasePtrs& base_ptrs,
               const LabelPtrs& label_ptrs,
               std::any& stats_any
           ) {
        auto& stats = std::any_cast<Stats&>(stats_any);
        if (stats.map > 0) {
            const auto& base_ptr = base_ptrs.at(0);
            auto game_ticks = read<uint32_t, int32_t>(
                handle, base_ptr + engine_offset, {0x38}
            );
            if (game_ticks) stats.time = game_ticks.value() * seconds_per_tick;
            return game_ticks.has_value();
        }
        return true;
    };
}