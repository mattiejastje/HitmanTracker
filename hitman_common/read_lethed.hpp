#pragma once

#include <cstdint>
#include <mempeep/read.hpp>
#include <optional>

#include "../hitman2_silent_assassin/structs.hpp"
#include "../mem/read_write.hpp"

template <IsTracer Tracer>
inline std::optional<uint32_t> read_lethed(
    uint32_t data,
    int32_t data_used,
    MemoryReader<uint32_t> reader,
    Tracer tracer
) {
    uint32_t offset = 0;
    uint32_t index = 0;  // to avoid infinite loop
    while (offset <= data_used || index++ < 0x200) {
        mempeep::RemoteValue<
            hitman2_silent_assassin::structs::TPropertyManagerRecord,
            uint32_t>
            remote_record{data + offset};
        hitman2_silent_assassin::structs::PropertyManagerRecord record{};
        if (!mempeep::read(remote_record, reader, tracer, record)) {
            logging::warn("Unable to read property manager record");
            return {};
        }
        // "lethed" property has record size 0x1C so filter on that first
        if (record.is_active && record.record_size == 0x1C) {
            hitman2_silent_assassin::structs::Property property{};
            if (!mempeep::read(record.property, reader, tracer, property)) {
                logging::warn("Unable to read property");
                return {};
            }
            if (property.key == "lethed") {
                if (property.size != 4) {
                    logging::warn("Property \"lethed\" has wrong size");
                    return {};
                }
                int32_t lethed;
                if (!reader(data + offset + 0x18, 4, &lethed)) {
                    logging::warn("Unable to read property \"lethed\" value");
                    return {};
                }
                return lethed;
            }
        }
        offset += record.record_size;
    }
    if (index == 0x200) logging::warn("Too many properties");
    return {};
}
