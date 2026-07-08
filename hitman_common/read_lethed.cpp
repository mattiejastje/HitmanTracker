#pragma once

#include "read_lethed.hpp"

#include "../hitman2_silent_assassin/structs.hpp"

std::optional<int32_t> hitman_common::read_property_int32(
    uint32_t data,
    int32_t data_used,
    std::string_view key,
    MemoryReader<uint32_t>& reader,
    LogTracer<MempeepOnLogEntry>& tracer
) {
    uint32_t offset = 0;
    uint32_t index = 0;  // to avoid infinite loop
    while (offset < data_used) {
        if (index++ > 0x200) {
            spdlog::warn("Too many properties");
            break;
        }
        mempeep::RemoteValue<
            hitman2_silent_assassin::structs::TPropertyManagerRecord,
            uint32_t>
            remote_record{data + offset};
        hitman2_silent_assassin::structs::PropertyManagerRecord record{};
        if (!mempeep::read(remote_record, reader, tracer, record)) {
            spdlog::warn("Unable to read property manager record");
            return {};
        }
        // record_size - 4 bytes
        // is_active - 1 byte
        // property.key_length - 4 bytes
        // property.type - 4 bytes
        // property.size - 4 bytes
        // property.key - ... (at least 1 byte)
        // so record_size > 4+1+4+4+4 = 0x11
        if (record.record_size <= 0x11) {
            spdlog::warn("Invalid property record size");
            return {};
        }
        // uint32_t property has record size
        // 0x11 (header) + key.size() + 1 (null) + 4 (value)
        // so filter on that first
        if (record.is_active
            && record.record_size == 0x11 + key.size() + 1 + 4) {
            hitman2_silent_assassin::structs::Property property{};
            if (!mempeep::read(record.property, reader, tracer, property)) {
                spdlog::warn("Unable to read property");
                return {};
            }
            if (property.key == key) {
                if (property.size != 4) {
                    spdlog::warn("Property \"{}\" has wrong size", key);
                    return {};
                }
                int32_t val;
                if (!reader(data + offset + 0x18, 4, &val)) {
                    spdlog::warn("Unable to read property \"{}\" value", key);
                    return {};
                }
                return val;
            }
        }
        offset += record.record_size;
    }
    return {};
}
