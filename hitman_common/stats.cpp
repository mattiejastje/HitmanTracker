#include "stats.hpp"

#include <array>
#include <cassert>
#include <set>
#include <vector>

#include "../logging.hpp"

static StatsArray stats_as_array(const Stats& stats) {
    return StatsArray{
        stats.shots_fired.value,
        stats.close_encounters.value,
        stats.headshots.value,
        stats.alerts.value,
        stats.enemies_killed.value,
        stats.enemies_wounded.value,
        stats.innocents_killed.value,
        stats.innocents_wounded.value,
    };
}

using StatsExcess = StatsArray;

// excess values above a silent assassin combination
// any values > 0 mean no silent assassin
static StatsExcess get_subtract(
    const StatsArray& stats, const StatsArray& stats_sa
) {
    StatsArray result{};
    for (int i = 0; i < 8; i++) {
        result[i] = stats[i] - stats_sa[i];
    };
    return result;
}

struct Excess {
    int32_t maximum;         // the maximum excess
    std::set<int> positive;  // strictly positive excess stats ("red")
};

static Excess get_arg_max(const StatsExcess& excess) {
    auto maximum = excess[0];
    for (int i = 1; i < 8; i++) {
        auto value = excess[i];
        if (maximum < value) maximum = value;
    };
    Excess arg_max{maximum, {}};
    for (int i = 0; i < 8; i++) {
        if (excess[i] > 0) arg_max.positive.insert(i);
    }
    return arg_max;
}

static std::vector<Excess> get_arg_maxs(
    const std::vector<StatsArray>& silent_assassin_combinations,
    const StatsArray& stats
) {
    std::vector<Excess> arg_maxs{};
    for (auto& stats_sa : silent_assassin_combinations) {
        arg_maxs.push_back(get_arg_max(get_subtract(stats, stats_sa)));
    }
    return arg_maxs;
}

static Excess get_arg_min(const std::vector<Excess>& arg_maxs) {
    assert(!arg_maxs.empty());
    Excess arg_min{arg_maxs[0]};
    for (auto& arg_max : arg_maxs) {
        if (arg_min.maximum > arg_max.maximum) {
            arg_min = arg_max;
        } else if (arg_min.maximum == arg_max.maximum) {
            for (auto i : arg_max.positive) arg_min.positive.insert(i);
        }
    }
    return arg_min;
};

static bool is_at_risk(
    const std::vector<StatsArray>& silent_assassin_combinations,
    const StatsArray& stats,
    int index
) {
    StatsArray stats_inc = stats;
    stats_inc[index] += 1;
    return get_arg_min(get_arg_maxs(silent_assassin_combinations, stats_inc))
        .positive.contains(index);
}

static Status get_status(
    const std::vector<StatsArray>& silent_assassin_combinations,
    const StatsArray& stats,
    int index,
    const Excess& arg_min
) {
    return arg_min.positive.contains(index) ? Status::RED
           : is_at_risk(silent_assassin_combinations, stats, index)
               ? Status::YELLOW
               : Status::GREEN;
};

void process_common_game_stats(
    const std::vector<StatsArray>& silent_assassin_combinations,
    const CommonGameStats& game_stats,
    Stats& stats
) {
    logging::trace("Headshots {}", game_stats.headshots);
    logging::trace("Enemies wounded {}", game_stats.enemies_wounded);
    logging::trace("Enemies killed {}", game_stats.enemies_killed);
    logging::trace("Innocents wounded {}", game_stats.innocents_wounded);
    logging::trace("Innocents killed {}", game_stats.innocents_killed);
    logging::trace("Alerts {}", game_stats.alerts);
    logging::trace("Close encounters {}", game_stats.close_encounters);
    stats.headshots.value = game_stats.headshots;
    stats.enemies_wounded.value = game_stats.enemies_wounded;
    stats.enemies_killed.value = game_stats.enemies_killed;
    stats.innocents_killed.value = game_stats.innocents_killed;
    stats.innocents_wounded.value = game_stats.innocents_wounded;
    stats.alerts.value = game_stats.alerts;
    stats.close_encounters.value = game_stats.close_encounters;
    auto stats_arr = stats_as_array(stats);
    auto arg_min
        = get_arg_min(get_arg_maxs(silent_assassin_combinations, stats_arr));
    stats.shots_fired.status
        = get_status(silent_assassin_combinations, stats_arr, 0, arg_min);
    stats.close_encounters.status
        = get_status(silent_assassin_combinations, stats_arr, 1, arg_min);
    stats.headshots.status
        = get_status(silent_assassin_combinations, stats_arr, 2, arg_min);
    stats.alerts.status
        = get_status(silent_assassin_combinations, stats_arr, 3, arg_min);
    stats.enemies_killed.status
        = get_status(silent_assassin_combinations, stats_arr, 4, arg_min);
    stats.enemies_wounded.status
        = get_status(silent_assassin_combinations, stats_arr, 5, arg_min);
    stats.innocents_killed.status
        = get_status(silent_assassin_combinations, stats_arr, 6, arg_min);
    stats.innocents_wounded.status
        = get_status(silent_assassin_combinations, stats_arr, 7, arg_min);
    auto rating = arg_min.maximum <= 0 ? Status::GREEN : Status::RED;
    stats.rating = {get_simple_rating_value(rating), rating};
}

std::string get_simple_rating_value(Status status) {
    return status == Status::GREEN ? "Silent Assassin" : "No Silent Assassin";
};
