#include "fnv1a.hpp"

#include <array>
#include <cstddef>
#include <fstream>
#include <unordered_map>

#include "logging.hpp"

uint64_t fnv1a::fnv1a(uint64_t hash, std::span<const std::byte> data) {
    for (std::byte b : data) {
        hash ^= std::to_integer<uint8_t>(b);
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

std::optional<uint64_t> fnv1a::fnv1a(const std::filesystem::path& path) {
    static std::unordered_map<std::filesystem::path, uint64_t> cache;
    auto it = cache.find(path);
    if (it != cache.end()) {
        logging::debug(
            "Using cached checksum  0x{:X} for {}", it->second, path.string()
        );
        return it->second;
    }
    logging::debug("Calculating checksum of {}", path.string());
    std::ifstream f(path, std::ios::binary);
    if (!f) return {};
    uint64_t hash = INITIAL_HASH;
    std::array<std::byte, 4096> buffer{};
    while (f) {
        f.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        std::streamsize count = f.gcount();
        if (count > 0) {
            logging::trace("Processing {} bytes for checksum", (int)count);
            hash = fnv1a(
                hash, std::span(buffer.data(), static_cast<size_t>(count))
            );
        }
    }
    if (!f.eof() && f.fail()) {
        logging::error("Failed to calculate checksum of {}", path.string());
        return {};
    }
    logging::debug("Checksum is 0x{:X}", hash);
    return hash;
}