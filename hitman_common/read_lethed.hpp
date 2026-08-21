#pragma once

#include <cstdint>
#include <mempeep/read.hpp>
#include <optional>
#include <string_view>

#include "../hitman2_silent_assassin/structs.hpp"
#include "../mem/read_write.hpp"
#include "stats.hpp"

namespace hitman_common {

std::optional<int32_t> read_property_int32(
    uint32_t data,
    int32_t data_used,
    std::string_view key,
    MemoryReader<uint32_t>& reader,
    LogTracer<MempeepOnLogEntry>& tracer,
    std::optional<PropertyCache>& cache
);

}
