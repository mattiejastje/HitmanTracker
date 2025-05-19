#include "stats.hpp"

#include <format>
#include <unordered_map>

#include "../hitman_common/stats.hpp"
#include "../logging.hpp"
#include "../mem/read_write.hpp"

static int32_t map_key(int32_t level, int32_t section) {
    return (level << 8) + section;
}

struct MapInfo {
    int map;
    int num_evidence;
    std::string rating_value;
};

const std::unordered_map<int32_t, MapInfo> scenes = {
    {map_key(-1, -1), {0}},                      // game loading
    {map_key(0, 0), {1, 0}},                     // garden
    {map_key(0, 1), {2, 0}},                     // greenhouse
    {map_key(0, 2), {3, 1}},                     // cliffside
    {map_key(0, 3), {4, 1, "Shadow"}},           // mansion ground
    {map_key(0, 4), {5, 0}},                     // mansion 2nd
    {map_key(1, 0), {6, 1, "Silent Assassin"}},  // king of chinatown
    {map_key(2, 0), {7, 1, "Shadow"}},           // terminus hotel
    {map_key(2, 1), {8, 1, "Shadow"}},           // upper floors
    // TODO (2, 2)
    {map_key(3, 0), {9, 0, "Specialist"}},         // burning hotel
    {map_key(3, 1), {10, 1, "Shadow"}},            // library
    {map_key(3, 3), {11, 0}},                      // pigeon coop
    {map_key(3, 4), {12, 1, "Shadow"}},            // shangri-la
    {map_key(3, 5), {13, 1, "Shadow"}},            // train station
    {map_key(4, 0), {14, 1, "Shadow"}},            // courtyard
    {map_key(4, 1), {15, 1, "Silent Assassin"}},   // vixen club
    {map_key(4, 2), {16, 0, "Specialist"}},        // dressing rooms
    {map_key(4, 3), {17, 1, "Shadow"}},            // derelict building
    {map_key(4, 4), {18, 1, "Shadow"}},            // convenience store
    {map_key(4, 5), {19, 1}},                      // loading area
    {map_key(4, 6), {20, 1, "Silent Assassin"}},   // chinese new year
    {map_key(6, 0), {21, 0}},                      // victoria's ward
    {map_key(6, 1), {22, 1, "Shadow"}},            // orphanage halls
    {map_key(6, 2), {23, 1, "Silent Assassin"}},   // central heating
    {map_key(7, 0), {24, 1, "Shadow"}},            // great balls of fire
    {map_key(8, 0), {25, 1, "Veteran"}},           // gunshop
    {map_key(9, 0), {26, 1, "Silent Assassin"}},   // streets of hope
    {map_key(9, 1), {27, 1, "Silent Assassin"}},   // barber shop
    {map_key(10, 0), {28, 0}},                     // the desert
    {map_key(11, 0), {29, 1, "Shadow"}},           // dead end
    {map_key(11, 1), {30, 1, "Shadow"}},           // old mill
    {map_key(11, 2), {31, 1, "Shadow"}},           // descent
    {map_key(11, 3), {32, 1, "Shadow"}},           // factory compound
    {map_key(12, 0), {33, 1, "Silent Assassin"}},  // test facility
    {map_key(12, 1), {34, 1, "Shadow"}},           // decontamination
    {map_key(12, 2), {35, 1, "Silent Assassin"}},  // r&d
    {map_key(13, 0), {36, 1, "Shadow"}},           // patriot's hangar
    {map_key(13, 1), {37, 1, "Silent Assassin"}},  // arena
    {map_key(14, 0), {38, 1, "Silent Assassin"}},  // parking
    {map_key(14, 1), {39, 1, "Silent Assassin"}},  // reception
    {map_key(14, 2), {40, 1, "Silent Assassin"}},  // cornfield
    {map_key(17, 0), {41, 1, "Shadow"}},           // courthouse
    {map_key(17, 1), {42, 1}},                     // holding cells
    {map_key(17, 2), {43, 1, "Shadow"}},           // prison
    {map_key(18, 0), {44, 1, "Shadow"}},           // county jail
    {map_key(18, 1), {45, 1, "Shadow"}},           // outgunned
    {map_key(18, 2), {46, 1, "Shadow"}},           // burn
    {map_key(18, 3), {47, 1, "Shadow"}},           // hope fair
    {map_key(18, 4), {48, 0}},                     // church
    {map_key(21, 0), {49, 1}},                     // tailor shop
    {map_key(22, 1), {50, 1, "Shadow"}},           // blackwater park
    {map_key(22, 2), {51, 1, "Silent Assassin"}},  // the penthouse
    {map_key(24, 0), {52, 1, "Silent Assassin"}},  // blackwater roof
    {map_key(25, 0), {53, 1, "Shadow"}},           // cemetary entrance
    {map_key(25, 1), {54, 1, "Silent Assassin"}},  // burnwood family tomb
    {map_key(25, 2), {55, 1, "Silent Assassin"}},  // crematorium
};

static Status get_rating_status(const Stats& stats) {
    return (stats.spotted.value != 0)         ? Status::RED
           : (stats.evidence_left.value != 0) ? Status::YELLOW
                                              : Status::GREEN;
};

static std::string get_rating_value(const MapInfo& map_info, Status status) {
    if (map_info.rating_value.empty()) {
        return "Unrated";
    }
    return std::format(
        "{}{}", status == Status::GREEN ? "" : "No ", map_info.rating_value
    );
}

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
        auto status = get_rating_status(stats);
        stats.rating = {get_simple_rating_value(status), status};
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
