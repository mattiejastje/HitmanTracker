#include "pe.hpp"

#include <fstream>

#include "logging.hpp"

std::optional<PeId> get_pe_id(const std::filesystem::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        logging::error("Failed to open {}", path.string());
        return {};
    }
    f.seekg(0x3C);
    uint32_t pe_offset{};
    f.read(reinterpret_cast<char*>(&pe_offset), sizeof(pe_offset));
    f.seekg(pe_offset + 4 + 4);
    uint32_t time_date_stamp{};
    f.read(reinterpret_cast<char*>(&time_date_stamp), sizeof(time_date_stamp));
    if (!f) {
        logging::error("Failed to read PE header of {}", path.string());
        return {};
    }
    logging::debug(
        "PE id for {}: timestamp={:#x}", path.string(), time_date_stamp
    );
    return PeId{time_date_stamp};
}