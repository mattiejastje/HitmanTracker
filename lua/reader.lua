local typedefs = require("typedefs")

local _readers = {}

-- ----------------------------------------------------------------------------
-- Dispatch
-- ----------------------------------------------------------------------------

local function read(addr, type_ref)
    if addr == nil then
        return nil, {"nil address"}
    end
    return _readers[type_ref.kind](addr, type_ref)
end

local function _prefix_errors(errors, prefix)
    local result = {}
    for _, err in ipairs(errors) do
        table.insert(result, prefix .. err)
    end
    return result
end

-- ----------------------------------------------------------------------------
-- Readers
-- ----------------------------------------------------------------------------

local sizer = require("sizer")

_readers.array = function(addr, type_ref)
    local sub_type = type_ref.type_ref
    local size = sizer.sizeof(sub_type)
    local obj = {}
    local errors = {}
    local p = addr
    for i = 1, type_ref.count do
        local val, errs = read(p, sub_type)
        if errs then
            for _, e in ipairs(_prefix_errors(errs, "[" .. i - 1 .. "]: ")) do
                table.insert(errors, e)
            end
        end
        table.insert(obj, val)
        p = p + size
    end
    return obj, next(errors) and errors or nil
end

_readers.circular_list = function(addr, type_ref)
    local sub_type = type_ref.type_ref
    local next_field = type_ref.next_field
    local obj = {}
    local errors = {}
    local head = readPointer(addr)
    if head == nil then
        return nil, {"Invalid head pointer"}
    elseif head ~= 0 then
        local p = head
        repeat
            local value, errs = read(p, sub_type)
            if errs then
                for _, e in ipairs(_prefix_errors(errs, "[" .. #obj .. "]: ")) do
                    table.insert(errors, e)
                end
            end
            table.insert(obj, value)
            if value then
                p = value[next_field]
                if p == nil then
                    table.insert(errors, "Invalid next pointer at index " .. #obj)
                    return obj, errors
                end
            else
                return obj, errors
            end
        until p == head
    end
    return obj, next(errors) and errors or nil
end

_readers.float = function(addr, type_ref)
    local v = readFloat(addr)
    if v == nil then return nil, {"Could not read float"} end
    return v
end

_readers.i8 = function(addr, type_ref)
    local v = readByte(addr, true)
    if v == nil then return nil, {"Could not read i8"} end
    return v
end

_readers.i16 = function(addr, type_ref)
    local v = readShortInteger(addr, true)
    if v == nil then return nil, {"Could not read i16"} end
    return v
end

_readers.i32 = function(addr, type_ref)
    local v = readInteger(addr, true)
    if v == nil then return nil, {"Could not read i32"} end
    return v
end

_readers.i64 = function(addr, type_ref)
    -- readQword returns a signed value
    local v = readQword(addr)
    if v == nil then return nil, {"Could not read i64"} end
    return v
end

_readers.ptr = function(addr, type_ref)
    local p = readPointer(addr)
    if p == nil then
        return nil, {"Could not read pointer"}
    end
    if p == 0 then
        if type_ref.optional then
            return nil
        end
        return nil, {"Null pointer"}
    end
    if type_ref.weak then
        return p
    end
    return read(p, type_ref.type_ref)
end

_readers.string = function(addr, type_ref)
    local s = readString(addr, type_ref.max_length)
    if s == nil then return nil, {"Could not read string"} end
    return s
end

_readers.struct = function(addr, type_ref)
    local def = typedefs.struct_defs[type_ref.name]
    if not def then
        error("Unknown struct: " .. type_ref.name)
    end
    local obj = {}
    local errors = {}
    for _, field in ipairs(def.fields) do
        local val, errs = read(addr + field.offset, field.type_ref)
        if errs then
            for _, e in ipairs(_prefix_errors(errs, field.name .. ": ")) do
                table.insert(errors, e)
            end
        end
        obj[field.name] = val
    end
    return obj, next(errors) and errors or nil
end

_readers.vector = function(addr, type_ref)
    local begin_ptr = readPointer(addr)
    local end_ptr = readPointer(addr + getPointerSize())
    local sub_type = type_ref.type_ref
    if begin_ptr == nil or end_ptr == nil then
        return nil, {"Invalid vector pointers"}
    end
    if begin_ptr > end_ptr then
        return nil, {"Vector begin pointer is past end pointer"}
    end
    local obj = {}
    local errors = {}
    local p = begin_ptr
    local size = sizer.sizeof(sub_type)
    local i = 0
    while p < end_ptr do
        local val, errs = read(p, sub_type)
        if errs then
            for _, e in ipairs(_prefix_errors(errs, "[" .. i .. "]: ")) do
                table.insert(errors, e)
            end
        end
        table.insert(obj, val)
        p = p + size
        i = i + 1
    end
    if p ~= end_ptr then
        table.insert(errors, "Vector pointer overflow")
    end
    return obj, next(errors) and errors or nil
end


-- ----------------------------------------------------------------------------
-- Public API
-- ----------------------------------------------------------------------------

local reader = {}

reader.read = read

return reader
