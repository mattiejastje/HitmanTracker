#include "stats.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

#include "../logging.hpp"
#include "../mem/read_write.hpp"

// OptionsScreen gets loaded during missions, assign negative value to ignore
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
    {R"(C1_HongKong\C1_3)", 4},
    {R"(C1_Hongkong\C1_3_Laptop)", 4},
    {R"(C1_HongKong\C1_4)", 5},
    {R"(C1_Hongkong\C1_4_Laptop)", 5},
    {R"(CutScenes/FlashBacks/FlashBack1_4)", 5},
    {R"(C4_ColombianRainforest\C4_1_Pre)", 6},
    {R"(C4_ColombianRainforest\C4_1)", 6},
    {R"(C4_ColombianRainForest\C4_1_Laptop)", 6},
    {R"(C4_ColombianRainforest\C4_2)", 7},
    {R"(C4_ColombianRainForest\C4_2_Laptop)", 7},
    {R"(C4_ColombianRainforest\C4_3)", 8},
    {R"(C4_ColombianRainForest\C4_3_Laptop)", 8},
    {R"(C3_BudapestHotel\C3_1)", 9},
    {R"(C3_BudapestHotel\C3_1_Laptop)", 9},
    {R"(CutScenes/FlashBacks/FlashBack3_1)", 9},
    {R"(C2_RotterdamHarbor\C2_1)", 10},
    {R"(C2_RotterdamHarbor\C2_1_Laptop)", 10},
    {R"(C2_RotterdamHarbor\C2_2)", 11},
    {R"(C2_RotterdamHarbor\C2_2_Laptop)", 11},
    {R"(CutScenes/FlashBacks/FlashBack2_2)", 11},
    {R"(C5_Sanitarium\C5_1)", 12},
    {R"(C5_Sanitarium\C5_1_Laptop)", 12},
    {R"(C5_Sanitarium\C5_2_Pre)", 13},
    {R"(C5_Sanitarium\C5_2)", 13},
};

bool hitman_codename_47::update_slow(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    auto difficulty = read<int32_t>(handle, label_ptrs.at(150));
    if (!difficulty) return false;
    stats.difficulty = difficulty.value();
    auto scene_head = read<int32_t>(
        handle, base_ptrs[1] + 0x1F000C, {0, 0x59, 0x7E, 0x1C}, INT32_MAX
    );
    auto scene_tail = read<int32_t>(
        handle, base_ptrs[1] + 0x1F000C, {0, 0x59, 0x7E, 0x20}, INT32_MAX
    );
    if (!scene_head || !scene_tail) return false;
    // tail = root scene (i.e. mission, main menu, options from main menu, ...)
    // head = child scene (i.e. laptop, options from mission, ...)
    auto scene = read_string(
        handle,
        base_ptrs[1] + 0x1F000C,
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
        auto scene_head = read<int32_t>(
            handle, base_ptrs[1] + 0x1F000C, {0, 0x59, 0x7E, 0x1C}, INT32_MAX
        );
        auto scene_tail = read<int32_t>(
            handle, base_ptrs[1] + 0x1F000C, {0, 0x59, 0x7E, 0x20}, INT32_MAX
        );
        if (!scene_head || !scene_tail) return false;
        std::optional<double> time = {};
        if (scene_head == scene_tail) {
            // main mission: use global game time
            time = read<double>(
                handle, base_ptrs[1] + 0x1F000C, {0, 0x37B5}, INT32_MAX
            );
        } else {
            // paused in the options menu: use time options menu was created
            time = read<double>(
                handle,
                base_ptrs[1] + 0x1F000C,
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
