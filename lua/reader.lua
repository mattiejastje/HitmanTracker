local typedefs = require("typedefs")

local _readers = {}

-- ----------------------------------------------------------------------------
-- Dispatch
-- ----------------------------------------------------------------------------

-- Returns {addr, value, error} always.
-- For containers, value is a table of {addr, value, error} elements.
local function read(addr, type_ref)
    if addr == nil then
        return {addr = nil, value = nil, error = "nil address"}
    end
    return _readers[type_ref.kind](addr, type_ref)
end

-- ----------------------------------------------------------------------------
-- Readers
-- ----------------------------------------------------------------------------

local sizer = require("sizer")

_readers.array = function(addr, type_ref)
    local sub_type = type_ref.type_ref
    local size = sizer.sizeof(sub_type)
    local items = {}
    local p = addr
    for i = 1, type_ref.count do
        table.insert(items, read(p, sub_type))
        p = p + size
    end
    return {addr = addr, value = items}
end

_readers.circular_list = function(addr, type_ref)
    local sub_type = type_ref.type_ref
    local next_field = type_ref.next_field
    local items = {}
    local head = readPointer(addr)
    if head == nil then
        return {addr = addr, value = nil, error = "Invalid head pointer"}
    elseif head ~= 0 then
        local p = head
        repeat
            local rawval = read(p, sub_type)
            table.insert(items, rawval)
            local struct_value = rawval.value
            if struct_value then
                local next_rawval = struct_value[next_field]
                local next_addr = next_rawval and next_rawval.value
                if next_addr == nil then
                    table.insert(items, {addr = p, value = nil, error = "Invalid next pointer"})
                    return {addr = addr, value = items}
                end
                p = next_addr
            else
                return {addr = addr, value = items}
            end
        until p == head
    end
    return {addr = addr, value = items}
end

_readers.float = function(addr, type_ref)
    local v = readFloat(addr)
    if v == nil then return {addr = addr, value = nil, error = "Could not read float"} end
    return {addr = addr, value = v}
end

_readers.i8 = function(addr, type_ref)
    local v = readByte(addr, true)
    if v == nil then return {addr = addr, value = nil, error = "Could not read i8"} end
    return {addr = addr, value = v}
end

_readers.i16 = function(addr, type_ref)
    local v = readShortInteger(addr, true)
    if v == nil then return {addr = addr, value = nil, error = "Could not read i16"} end
    return {addr = addr, value = v}
end

_readers.i32 = function(addr, type_ref)
    local v = readInteger(addr, true)
    if v == nil then return {addr = addr, value = nil, error = "Could not read i32"} end
    return {addr = addr, value = v}
end

_readers.i64 = function(addr, type_ref)
    local v = readQword(addr)
    if v == nil then return {addr = addr, value = nil, error = "Could not read i64"} end
    return {addr = addr, value = v}
end

_readers.ptr = function(addr, type_ref)
    local p = readPointer(addr)
    if p == nil then
        return {addr = addr, value = nil, error = "Could not read pointer"}
    end
    if p == 0 then
        if type_ref.optional then
            return {addr = addr, value = nil}
        end
        return {addr = addr, value = nil, error = "Null pointer"}
    end
    if type_ref.weak then
        return {addr = addr, value = p}
    end
    return {addr = addr, value = read(p, type_ref.type_ref)}
end

_readers.string = function(addr, type_ref)
    local s = readString(addr, type_ref.max_length)
    if s == nil then return {addr = addr, value = nil, error = "Could not read string"} end
    return {addr = addr, value = s}
end

_readers.struct = function(addr, type_ref)
    local def = typedefs.struct_defs[type_ref.name]
    if not def then
        error("Unknown struct: " .. type_ref.name)
    end
    local obj = {}
    for _, field in ipairs(def.fields) do
        obj[field.name] = read(addr + field.offset, field.type_ref)
    end
    return {addr = addr, value = obj}
end

_readers.vector = function(addr, type_ref)
    local begin_ptr = readPointer(addr)
    local end_ptr = readPointer(addr + getPointerSize())
    local sub_type = type_ref.type_ref
    if begin_ptr == nil or end_ptr == nil then
        return {addr = addr, value = nil, error = "Invalid vector pointers"}
    end
    if begin_ptr > end_ptr then
        return {addr = addr, value = nil, error = "Vector begin pointer is past end pointer"}
    end
    local items = {}
    local p = begin_ptr
    local size = sizer.sizeof(sub_type)
    while p < end_ptr do
        table.insert(items, read(p, sub_type))
        p = p + size
    end
    if p ~= end_ptr then
        table.insert(items, {addr = p, value = nil, error = "Vector pointer overflow"})
    end
    return {addr = addr, value = items}
end


-- ----------------------------------------------------------------------------
-- Public API
-- ----------------------------------------------------------------------------

local reader = {}

reader.read = read

return reader
