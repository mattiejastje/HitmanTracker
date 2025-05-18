#include "stats.hpp"

#include <unordered_map>

#include "../logging.hpp"
#include "../mem/read_write.hpp"

static int32_t map_key(int32_t level, int32_t section) {
    return (level << 8) + section;
}

struct MapInfo {
    int map;
    int num_evidence;
};

const std::unordered_map<int32_t, MapInfo> scenes = {
    {map_key(-1, -1), {0}},     // game loading
    {map_key(25, 0), {0}},      // menu
    {map_key(0, 0), {1, 0}},    // garden
    {map_key(0, 1), {2, 0}},    // greenhouse
    {map_key(0, 2), {3, 1}},    // cliffside
    {map_key(0, 3), {4, 1}},    // mansion ground
    {map_key(0, 4), {5, 0}},    // mansion 2nd
    {map_key(1, 0), {6, 1}},    // king of chinatown
    {map_key(2, 0), {7, 1}},    // terminus hotel
    {map_key(2, 1), {8, 1}},    // upper floors
    {map_key(3, 0), {9, 0}},    // burning hotel
    {map_key(3, 1), {10, 1}},   // library
    {map_key(3, 3), {11, 0}},   // pigeon coop
    {map_key(3, 4), {12, 1}},   // shangri'la
    {map_key(3, 5), {13, 1}},   // train station
    {map_key(4, 0), {14, 1}},   // courtyard
    {map_key(4, 1), {15, 1}},   // vixen club
    {map_key(4, 2), {16, 0}},   // dressing rooms
    {map_key(4, 3), {17, 1}},   // derelict building
    {map_key(4, 4), {18, 1}},   // convenience store
    {map_key(4, 5), {19, 1}},   // loading area
    {map_key(4, 6), {20, 1}},   // chinese new year
    {map_key(6, 0), {21, 0}},   // victoria's ward
    {map_key(6, 1), {22, 1}},   // orphanage halls
    {map_key(6, 2), {23, 1}},   // central heating
    {map_key(7, 0), {24, 1}},   // great balls of fire
    {map_key(8, 0), {25, 1}},   // gunshop
    {map_key(9, 0), {26, 1}},   // streets of hope
    {map_key(9, 1), {27, 1}},   // barber shop
    {map_key(10, 0), {28, 0}},  // the desert
    {map_key(11, 0), {29, 1}},  // dead end
    {map_key(11, 1), {30, 1}},  // old mill
    {map_key(11, 2), {31, 1}},  // descent
    {map_key(11, 3), {32, 1}},  // factory compound
    {map_key(12, 0), {33, 1}},  // test facility
    {map_key(12, 1), {34, 1}},  // decontamination
    {map_key(12, 2), {35, 1}},  // r&d
    {map_key(13, 0), {36, 1}},  // patriot's hangar
    {map_key(13, 1), {37, 1}},  // arena
    {map_key(14, 0), {38, 1}},  // parking
    {map_key(14, 1), {39, 1}},  // reception
    {map_key(14, 2), {40, 1}},  // cornfield
    {map_key(17, 0), {41, 1}},  // courthouse
    {map_key(17, 1), {42, 1}},  // holding cells
    {map_key(17, 2), {43, 1}},  // prison
    {map_key(18, 0), {44, 1}},  // county jail
    {map_key(18, 1), {45, 1}},  // outgunned
    {map_key(18, 2), {46, 1}},  // burn
    {map_key(18, 3), {47, 1}},  // hope fair
    {map_key(18, 4), {48, 0}},  // church
    {map_key(21, 0), {49, 1}},  // tailor shop
    {map_key(22, 1), {50, 1}},  // blackwater park
    {map_key(22, 2), {51, 1}},  // the penthouse
    {map_key(24, 0), {52, 1}},  // blackwater roof
    {map_key(25, 0), {53, 1}},  // cemetary entrance
    {map_key(25, 1), {54, 1}},  // burnwood family tomb
    {map_key(25, 2), {55, 1}},  // crematorium
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
