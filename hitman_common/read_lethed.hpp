#pragma once

#include <cstdint>
#include <mempeep/read.hpp>
#include <optional>

#include "../hitman2_silent_assassin/structs.hpp"
#include "../mem/read_write.hpp"

namespace hitman_common {

std::optional<uint32_t> read_lethed(
    uint32_t data,
    int32_t data_used,
    MemoryReader<uint32_t>& reader,
    LogTracer<MempeepOnLogEntry>& tracer
);

}
