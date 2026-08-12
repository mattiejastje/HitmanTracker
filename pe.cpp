#include "pe.hpp"

#include <spdlog/spdlog.h>
#include <windows.h>

#include <fstream>

std::optional<PeId> get_pe_id(const std::filesystem::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        spdlog::error("Failed to open {}", path.string());
        return {};
    }
    uint16_t mz_sig{};
    f.read(reinterpret_cast<char*>(&mz_sig), sizeof(mz_sig));
    if (!f || mz_sig != IMAGE_DOS_SIGNATURE) {
        spdlog::error("Missing MZ signature: {}", path.string());
        return {};
    }
    f.seekg(0x3C);
    uint32_t pe_offset{};
    f.read(reinterpret_cast<char*>(&pe_offset), sizeof(pe_offset));
    if (!f) {
        spdlog::error("Failed to read PE header offset of {}", path.string());
        return {};
    }
    f.seekg(pe_offset);
    uint32_t pe_sig{};
    f.read(reinterpret_cast<char*>(&pe_sig), sizeof(pe_sig));
    if (!f || pe_sig != IMAGE_NT_SIGNATURE) {
        spdlog::error("Missing PE signature: {}", path.string());
        return {};
    }
    f.seekg(pe_offset + 4 + 4);
    uint32_t time_date_stamp{};
    f.read(reinterpret_cast<char*>(&time_date_stamp), sizeof(time_date_stamp));
    if (!f) {
        spdlog::error("Failed to read time date stamp of {}", path.string());
        return {};
    }
    spdlog::debug(
        "PE id for {}: timestamp={:#x}", path.string(), time_date_stamp
    );
    return PeId{time_date_stamp};
}