#include "game_info.hpp"

std::vector<GameInfo>& get_game_info_registry() {
    static std::vector<GameInfo> registry;
    return registry;
}
