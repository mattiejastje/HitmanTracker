#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>

namespace fnv1a {

inline constexpr uint64_t INITIAL_HASH = 0xcbf29ce484222325ULL;

uint64_t fnv1a(uint64_t hash, std::span<const std::byte> data);

std::optional<uint64_t> fnv1a(const std::filesystem::path& path);

}  // namespace fnv1a