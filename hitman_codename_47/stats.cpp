#include "stats.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <regex>
#include <string>
#include <unordered_map>

#include "../logging.hpp"
#include "../mem/read_write.hpp"

const std::unordered_map<std::string, int> scenes = {
    {R"(OptionsScreen)", 0},              // options
    {R"(OptionsScreen.zip)", 0},          // options
    {R"(Intro.zip)", 0},                  // menu
    {R"(CutScenes/Intro/Intro.zip)", 0},  // menu
    {R"(Intro)", 0},                      // menu (after beating game)
    {R"(C0_Training\C0_1_Pre)", 1},
    {R"(C0_Training\C0_1)", 1},
    {R"(C1_HongKong\C1_1_Pre)", 2},
    {R"(C1_HongKong\C1_1)", 2},
    {R"(CutScenes/C1_HongKong/C1_1_HitmanArrive.zip)", 2},
    {R"(C1_Hongkong\C1_1_Laptop)", 2},
    {R"(C1_HongKong\C1_2_Pre)", 3},
    {R"(C1_HongKong\C1_2)", 3},
    {R"(C1_Hongkong\C1_2_Laptop)", 3},
    {R"(C1_HongKong\C1_3_Pre)", 4},
    {R"(C1_HongKong\C1_3)", 4},
    {R"(C1_Hongkong\C1_3_Laptop)", 4},
    {R"(C1_HongKong\C1_4_Pre)", 5},
    {R"(C1_HongKong\C1_4)", 5},
    {R"(C1_Hongkong\C1_4_Laptop)", 5},
    {R"(CutScenes/FlashBacks/FlashBack1_4)", 5},
    {R"(C4_ColombianRainforest\C4_1_Pre)", 6},
    {R"(C4_ColombianRainforest\C4_1)", 6},
    {R"(C4_ColombianRainForest\C4_1_Laptop)", 6},
    {R"(C4_ColombianRainforest\C4_2_Pre)", 7},
    {R"(C4_ColombianRainforest\C4_2)", 7},
    {R"(C4_ColombianRainForest\C4_2_Laptop)", 7},
    {R"(C4_ColombianRainforest\C4_3_Pre)", 8},
    {R"(C4_ColombianRainforest\C4_3)", 8},
    {R"(C4_ColombianRainForest\C4_3_Laptop)", 8},
    {R"(C3_BudapestHotel\C3_1_Pre)", 9},
    {R"(C3_BudapestHotel\C3_1)", 9},
    {R"(C3_BudapestHotel\C3_1_Laptop)", 9},
    {R"(CutScenes/FlashBacks/FlashBack3_1)", 9},
    {R"(C2_RotterdamHarbor\C2_1_Pre)", 10},
    {R"(C2_RotterdamHarbor\C2_1)", 10},
    {R"(C2_RotterdamHarbor\C2_1_Laptop)", 10},
    {R"(C2_RotterdamHarbor\C2_2_Pre)", 11},
    {R"(C2_RotterdamHarbor\C2_2)", 11},
    {R"(C2_RotterdamHarbor\C2_2_Laptop)", 11},
    {R"(CutScenes/FlashBacks/FlashBack2_2)", 11},
    {R"(C5_Sanitarium\C5_1_Pre)", 12},
    {R"(C5_Sanitarium\C5_1)", 12},
    {R"(C5_Sanitarium\C5_1_Laptop)", 12},
    {R"(C5_Sanitarium\C5_2_Pre)", 13},
    {R"(C5_Sanitarium\C5_2)", 13},
};

struct Random {
    uint32_t state;

    explicit Random(uint32_t seed) : state(seed) {}

    uint16_t next() {
        state = state * 69069u + 1u;
        return static_cast<uint16_t>((state >> 8) & 0x7FFF);
    }
};

static uint32_t checksum(const char* data, size_t len) {
    uint32_t acc = 0;
    size_t word_count = len / 2;
    for (size_t i = 0; i < word_count; i++) {
        int16_t word;
        memcpy(&word, data + i * 2, 2);
        // cast to int32_t to ensure sign is extended
        // then cast to unsigned to ensure correct type for shifting
        acc ^= static_cast<uint32_t>(static_cast<int32_t>(word));
        if (acc & 0x110) {
            acc = (acc << 1) | (acc >> 31);
        } else {
            acc = (acc >> 1) | (acc << 31);
        }
    }
    return acc;
}

static std::optional<std::string> read_hitman_sav(
    const std::filesystem::path& path
) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) {
        logging::error("Failed to open save file: {}", path.string());
        return {};
    }
    long file_size = f.tellg();
    f.seekg(0);
    if (file_size < 12) {
        logging::error("File too small to be a valid save");
        return {};
    }
    std::vector<uint8_t> raw(file_size);
    if (!f.read(reinterpret_cast<char*>(raw.data()), file_size)) {
        logging::error("Failed to read save file");
        return {};
    }
    if (memcmp(raw.data(), "JPMH", 4) != 0) {
        logging::error("Bad magic: not a Hitman save file");
        return {};
    }
    uint32_t stored_checksum;
    memcpy(&stored_checksum, raw.data() + 4, 4);
    uint32_t seed;
    memcpy(&seed, raw.data() + 8, 4);
    const size_t header_size = 12;
    size_t payload_size = file_size - header_size;
    std::string payload(
        reinterpret_cast<char*>(raw.data()) + header_size, payload_size
    );
    Random rand(seed);
    size_t word_count = payload_size / 2;
    for (size_t i = 0; i < word_count; i++) {
        uint16_t key = rand.next();
        uint16_t word;
        memcpy(&word, payload.data() + i * 2, 2);
        word ^= key;
        memcpy(payload.data() + i * 2, &word, 2);
    }
    if (payload_size % 2 == 1) {
        uint16_t key = rand.next();
        payload[payload_size - 1] ^= static_cast<uint8_t>(key & 0xFF);
    }
    uint32_t calculated_checksum = checksum(payload.data(), payload_size);
    if (calculated_checksum != stored_checksum) {
        logging::error("Checksum mismatch: save file may be corrupt");
        return {};
    }
    return payload;
}

static std::optional<int32_t> find_difficulty(const std::string& payload) {
    std::regex re(" Difficulty=\"(\\d+)\" Current=\"1\"");
    std::smatch match;
    if (!std::regex_search(payload, match, re)) {
        logging::warn("Difficulty could not be identified from save file");
        return {};
    }
    return std::stoi(match[1]);
}

static std::optional<int32_t> read_difficulty_from_hitman_sav(
    const std::filesystem::path& path
) {
    static std::filesystem::file_time_type cached_mtime{};
    static std::optional<int32_t> cached_difficulty{};
    std::error_code ec;
    auto mtime = std::filesystem::last_write_time(path, ec);
    if (ec) {
        logging::error(
            "Unable to check last write time of {}: {}",
            path.string(),
            ec.message()
        );
        return {};
    }
    if (mtime == cached_mtime) return cached_difficulty;
    logging::debug("Reading Hitman.sav");
    cached_mtime = mtime;
    auto payload = read_hitman_sav(path);
    if (!payload) return {};
    logging::trace("Hitman.sav:\n{}", *payload);
    cached_difficulty = find_difficulty(*payload);
    return cached_difficulty;
}

bool hitman_codename_47::update_slow(
    const std::filesystem::path& exe_path,
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    const auto& base_ptr = base_ptrs.at(1);  // hitmandlc.dlc
    const auto hitman_sav = exe_path.parent_path() / "Hitman.sav";
    stats.difficulty = read_difficulty_from_hitman_sav(hitman_sav).value_or(0);
    auto scene_head = read<int32_t>(
        handle, base_ptr + 0x1F000C, {0, 0x59, 0x7E, 0x1C}, INT32_MAX
    );
    auto scene_tail = read<int32_t>(
        handle, base_ptr + 0x1F000C, {0, 0x59, 0x7E, 0x20}, INT32_MAX
    );
    if (!scene_head || !scene_tail) return false;
    // tail = root scene (i.e. mission, main menu, options from main menu, ...)
    // head = child scene (i.e. laptop, options from mission, ...)
    auto scene = read_string(
        handle,
        base_ptr + 0x1F000C,
        {0, 0x59, 0x7E, 0x20, -0x106, 0x0, 0x0},
        INT32_MAX,
        64
    );
    if (!scene) return false;
    logging::trace("Scene {}", scene.value());
    auto iter = scenes.find(scene.value());
    if (iter != scenes.end()) {
        stats.map = iter->second;
        stats.map_stage = MapStage::main;
        logging::trace("Map {}", stats.map);
    } else {
        if (!scene.value().empty()) {
            logging::error("No map registered for scene {}", scene.value());
        }
    }
    return true;
}

bool hitman_codename_47::update_fast(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    if (stats.map > 0) {
        const auto& base_ptr = base_ptrs.at(1);  // hitmandlc.dlc
        auto scene_head = read<int32_t>(
            handle, base_ptr + 0x1F000C, {0, 0x59, 0x7E, 0x1C}, INT32_MAX
        );
        auto scene_tail = read<int32_t>(
            handle, base_ptr + 0x1F000C, {0, 0x59, 0x7E, 0x20}, INT32_MAX
        );
        if (!scene_head || !scene_tail) return false;
        std::optional<double> time = {};
        if (scene_head == scene_tail) {
            // main mission: use global game time
            time = read<double>(
                handle, base_ptr + 0x1F000C, {0, 0x37B5}, INT32_MAX
            );
        } else {
            // paused in the options menu: use time options menu was created
            time = read<double>(
                handle,
                base_ptr + 0x1F000C,
                {0, 0x59, 0x7E, 0x1C, -0x108 + 0xD6},
                INT32_MAX
            );
        }
        if (!time) return false;
        stats.time = time.value();
        return true;
    }
    return true;
}
