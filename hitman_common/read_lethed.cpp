#include "read_lethed.hpp"

#include "../hitman2_silent_assassin/structs.hpp"

struct PropertyRecordResult {
    std::optional<int32_t> value;
    int32_t record_size;
};

static std::optional<PropertyRecordResult> read_property_record(
    uint32_t address,
    std::string_view key,
    MemoryReader<uint32_t>& reader,
    LogTracer<MempeepOnLogEntry>& tracer
) {
    mempeep::RemoteValue<
        hitman2_silent_assassin::structs::TPropertyManagerRecord,
        uint32_t>
        remote_record{address};
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
    if (record.is_active && record.record_size == 0x11 + key.size() + 1 + 4) {
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
            if (!reader(address + 0x11 + key.size() + 1, 4, &val)) {
                spdlog::warn("Unable to read property \"{}\" value", key);
                return {};
            }
            return PropertyRecordResult{
                .value = val, .record_size = record.record_size
            };
        }
    }
    return PropertyRecordResult{
        .value = std::nullopt, .record_size = record.record_size
    };
}

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
        auto result = read_property_record(data + offset, key, reader, tracer);
        if (!result) return {};
        if (result->value) return result->value;
        offset += result->record_size;
    }
    return {};
}
