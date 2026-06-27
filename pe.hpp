#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>

// unique identifier for executable
// to distinguish between versions of the same game (e.g. GOG, Steam, ...)
// if ever not enough, we might add more fields
struct PeId {
    uint32_t time_date_stamp;
};

std::optional<PeId> get_pe_id(const std::filesystem::path& path);
