#include "stats.hpp"

#include <spdlog/spdlog.h>

#include <array>
#include <cassert>
#include <cmath>
#include <format>
#include <set>
#include <vector>

#include "simple_rating.hpp"

using namespace hitman_common;

static bool is_at_risk(
    StatsFunc aggression,
    StatsFunc stealth,
    const StatsArray<int32_t>& stats,
    int index
) {
    StatsArray<int32_t> stats_inc = stats;
    stats_inc[index] += 1;
    return aggression(stats_inc) > 6 || stealth(stats_inc) > 1;
}

// both games use same weights
constexpr StatsArray<int> weights = {
    2,  // SHOTS_FIRED
    1,  // CLOSE_ENCOUNTERS
    1,  // HEADSHOTS
    1,  // ALERTS
    3,  // ENEMIES_KILLED
    1,  // ENEMIES_WOUNDED
    6,  // INNOCENTS_KILLED
    3,  // INNOCENTS_WOUNDED
};

// heuristic greedy hill-climber algorithm
static StatsArray<int32_t> find_nearest_safe_stats(
    StatsFunc measure_aggression,
    StatsFunc measure_stealth,
    const StatsArray<int32_t>& stats
) {
    StatsArray<int32_t> nearest = stats;
    while (true) {
        const auto current1 = measure_aggression(nearest);
        const auto current2 = measure_stealth(nearest);
        const auto v1 = std::max(current1 - 6, 0);
        const auto v2 = std::max(current2 - 1, 0);
        if (v1 == 0 && v2 == 0) break;
        int best_index = -1;
        double best_gain = -std::numeric_limits<double>::infinity();
        // for each index, check which gives best improvement
        for (int i = 0; i < 8; i++) {
            if (nearest[i] == 0) continue;
            nearest[i]--;
            const auto d1 = current1 - measure_aggression(nearest);
            const auto d2 = current2 - measure_stealth(nearest);
            nearest[i]++;
            const auto raw_gain = v1 * d1 + v2 * d2;
            const double gain = static_cast<double>(raw_gain) / weights[i];
            if (gain > best_gain) {
                best_gain = gain;
                best_index = i;
            }
        }
        if (best_index == -1) {
            // should never happen, but catch just in case to prevent infinite
            // loop
            spdlog::error("No best index found for nearest safe stats");
            break;
        };
        nearest[best_index]--;
    }
    spdlog::trace("safe shots_fired       = {}", nearest[SHOTS_FIRED]);
    spdlog::trace("safe close_encounters  = {}", nearest[CLOSE_ENCOUNTERS]);
    spdlog::trace("safe headshots         = {}", nearest[HEADSHOTS]);
    spdlog::trace("safe alerts            = {}", nearest[ALERTS]);
    spdlog::trace("safe enemies_killed    = {}", nearest[ENEMIES_KILLED]);
    spdlog::trace("safe enemies_wounded   = {}", nearest[ENEMIES_WOUNDED]);
    spdlog::trace("safe innocents_killed  = {}", nearest[INNOCENTS_KILLED]);
    spdlog::trace("safe innocents_wounded = {}", nearest[INNOCENTS_WOUNDED]);
    return nearest;
}

static StatsArray<Status> find_common_stats_status(
    StatsFunc aggression, StatsFunc stealth, const StatsArray<int32_t>& stats
) {
    auto safe_stats = find_nearest_safe_stats(aggression, stealth, stats);
    StatsArray<Status> status{};
    for (int i = 0; i < 8; i++) {
        status[i] = stats[i] > safe_stats[i]                    ? Status::RED
                    : is_at_risk(aggression, stealth, stats, i) ? Status::YELLOW
                                                                : Status::GREEN;
    }
    return status;
}

void hitman_common::process_game_stats(
    StatsFunc measure_aggression,
    StatsFunc measure_stealth,
    const StatsArray<int32_t> game_stats,
    Stats& stats
) {
    auto game_status = find_common_stats_status(
        measure_aggression, measure_stealth, game_stats
    );
    stats.shots_fired = {game_stats[SHOTS_FIRED], game_status[SHOTS_FIRED]};
    stats.close_encounters
        = {game_stats[CLOSE_ENCOUNTERS], game_status[CLOSE_ENCOUNTERS]};
    stats.headshots = {game_stats[HEADSHOTS], game_status[HEADSHOTS]};
    stats.alerts = {game_stats[ALERTS], game_status[ALERTS]};
    stats.enemies_killed
        = {game_stats[ENEMIES_KILLED], game_status[ENEMIES_KILLED]};
    stats.enemies_wounded
        = {game_stats[ENEMIES_WOUNDED], game_status[ENEMIES_WOUNDED]};
    stats.innocents_killed
        = {game_stats[INNOCENTS_KILLED], game_status[INNOCENTS_KILLED]};
    stats.innocents_wounded
        = {game_stats[INNOCENTS_WOUNDED], game_status[INNOCENTS_WOUNDED]};
    auto stealth = measure_stealth(game_stats);
    stats.stealth
        = {std::format("{:.3g}", 100 * std::pow(0.9, stealth)),
           stealth == 0   ? Status::GREEN
           : stealth == 1 ? Status::YELLOW
                          : Status::RED};
    auto aggression = measure_aggression(game_stats);
    stats.aggression
        = {std::format("{:.3g}", 100 * std::tanh(0.005 * aggression)),
           aggression <= 5   ? Status::GREEN
           : aggression == 6 ? Status::YELLOW
                             : Status::RED};
    auto rating_status
        = aggression <= 6 && stealth <= 1 ? Status::GREEN : Status::RED;
    stats.rating = {get_simple_rating_value(rating_status), rating_status};
};