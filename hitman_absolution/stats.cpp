#include "stats.hpp"

#include <unordered_map>

#include "../logging.hpp"
#include "../mem/read_write.hpp"

static int32_t map_key(int32_t level, int32_t section) {
    return (level << 8) + section;
}

enum class MaxRating { unrated, specialist, shadow, silent_assassin };

struct MapInfo {
    int map;
    int num_evidence;
    MaxRating max_rating;
};

const std::unordered_map<int32_t, MapInfo> scenes = {
    {map_key(-1, -1), {0}},                               // game loading
    {map_key(25, 0), {0}},                                // menu
    {map_key(0, 0), {1, 0, MaxRating::unrated}},          // garden
    {map_key(0, 1), {2, 0, MaxRating::unrated}},          // greenhouse
    {map_key(0, 2), {3, 1, MaxRating::unrated}},          // cliffside
    {map_key(0, 3), {4, 1, MaxRating::shadow}},           // mansion ground
    {map_key(0, 4), {5, 0, MaxRating::unrated}},          // mansion 2nd
    {map_key(1, 0), {6, 1, MaxRating::silent_assassin}},  // king of chinatown
    {map_key(2, 0), {7, 1, MaxRating::shadow}},           // terminus hotel
    {map_key(2, 1), {8, 1, MaxRating::shadow}},           // upper floors
    {map_key(3, 0), {9, 0, MaxRating::shadow}},           // burning hotel
    {map_key(3, 1), {10, 1, MaxRating::shadow}},          // library
    {map_key(3, 3), {11, 0, MaxRating::shadow}},          // pigeon coop
    {map_key(3, 4), {12, 1, MaxRating::shadow}},          // shangri'la
    {map_key(3, 5), {13, 1, MaxRating::shadow}},          // train station
    {map_key(4, 0), {14}},
    {map_key(4, 1), {15}},
    {map_key(4, 2), {16}},
    {map_key(4, 3), {17}},
    {map_key(4, 4), {18}},
    {map_key(4, 5), {19}},
    {map_key(4, 6), {20}},
    {map_key(6, 0), {21}},
    {map_key(6, 1), {22}},
    {map_key(6, 2), {23}},
    {map_key(7, 0), {24}},
    {map_key(8, 0), {25}},
    {map_key(9, 0), {26}},
    {map_key(9, 1), {27}},
    {map_key(10, 0), {28}},
    {map_key(11, 2), {29}},
    {map_key(11, 3), {30}},
    {map_key(11, 4), {31}},
    // map 32 is also (11, 4)?
    {map_key(12, 1), {33}},
    {map_key(12, 2), {34}},
    // map 35 is also (12, 2)?
    {map_key(13, 1), {36}},
    // map 37 is also (13, 1)?
    {map_key(14, 1), {38}},
    {map_key(14, 2), {39}},
    {map_key(14, 3), {40}},
    {map_key(17, 1), {41}},
    {map_key(17, 2), {42}},
    {map_key(17, 3), {43}},
    {map_key(18, 1), {44}},
    {map_key(18, 2), {45}},
    {map_key(18, 3), {46}},
    {map_key(18, 5), {47}},
    // map 48?
    {map_key(21, 0), {49}},
    {map_key(22, 1), {50}},
    {map_key(22, 2), {51}},
    {map_key(24, 0), {52}},
    {map_key(25, 1), {53}},
    {map_key(25, 2), {54}},
    // map 55 is also (25, 2)?
};

static Status get_silent_assassin(const Stats& stats) {
    return (stats.spotted.value != 0)         ? Status::RED
           : (stats.evidence_left.value != 0) ? Status::YELLOW
                                              : Status::GREEN;
};

void hitman_absolution::update_slow(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    auto level = read<int32_t>(handle, base_ptrs[0] + 0xE20F48);
    auto section = read<int32_t>(handle, base_ptrs[0] + 0xD60F94);
    if (!level) {
        logging::error("Unable to read level");
        return;
    }
    if (!section) {
        logging::error("Unable to read section");
        return;
    }
    auto key = map_key(level.value(), section.value());
    logging::trace(
        "Level {}, section {}, key {}", level.value(), section.value(), key
    );
    auto iter = scenes.find(key);
    if (iter != scenes.end()) {
        stats.map = iter->second.map;
        logging::trace("Map {}", stats.map);
    } else {
        logging::error(
            "No map registered for level {} section {}",
            level.value(),
            section.value()
        );
    }
    stats.map_stage = MapStage::main;  // always render stats
    if (stats.map > 0) {
        auto spotted = read<int32_t>(handle, base_ptrs[0] + 0xD61568);
        if (spotted) {
            stats.spotted = stats_value(spotted.value());
        }
        auto evidence_collected
            = read<int32_t>(handle, base_ptrs[0] + 0xE20FB0, {0x9C});
        if (evidence_collected) {
            stats.evidence_left = stats_value(
                iter->second.num_evidence - evidence_collected.value()
            );
        }
        stats.silent_assassin = get_silent_assassin(stats);
    }
}

void hitman_absolution::update_fast(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    if (stats.map > 0) {
        auto time = read<int32_t>(handle, base_ptrs[0] + 0xE24640);
        if (time) {
            stats.time = time.value() * 9.5367431640625E-7f;  // 1 / 1024 / 1024
        } else {
            logging::error("Unable to read time");
        }
    }
}
