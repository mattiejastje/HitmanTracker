#include "stats.hpp"

#include "../read_process_memory.hpp"

void stats_hitman2_silent_assassin(
    const ProcessHandlePtr& handle, Stats& stats
) {
    auto map_key
        = read_string(handle, (const void*)0x006A6C5C, {0x98, 0xBC7}, 8);
}
