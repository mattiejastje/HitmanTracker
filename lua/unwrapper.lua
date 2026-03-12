local typedefs = require("typedefs")

local _unwrappers = {}

-- ----------------------------------------------------------------------------
-- Dispatch
-- ----------------------------------------------------------------------------

local function unwrap(type_ref, rawval)
    if rawval == nil or rawval.value == nil then
        return nil
    end

    local unwrapper = _unwrappers[type_ref.kind]
    if not unwrapper then
        error("Unknown type kind: " .. type_ref.kind)
    end
    return unwrapper(type_ref, rawval.value)
end

-- ----------------------------------------------------------------------------
-- Unwrappers
-- ----------------------------------------------------------------------------

_unwrappers.array = function(type_ref, value)
    local result = {}
    for i, elem in ipairs(value) do
        result[i] = unwrap(type_ref.type_ref, elem)
    end
    return result
end

_unwrappers.circular_list = function(type_ref, value)
    local result = {}
    for i, elem in ipairs(value) do
        result[i] = unwrap(type_ref.type_ref, elem)
    end
    return result
end

_unwrappers.float = function(type_ref, value)
    return value
end

_unwrappers.i8 = function(type_ref, value)
    return value
end

_unwrappers.i16 = function(type_ref, value)
    return value
end

_unwrappers.i32 = function(type_ref, value)
    return value
end

_unwrappers.i64 = function(type_ref, value)
    return value
end

_unwrappers.ptr = function(type_ref, value)
    if type_ref.weak then
        return value
    end
    return unwrap(type_ref.type_ref, value)
end

_unwrappers.string = function(type_ref, value)
    return value
end

_unwrappers.struct = function(type_ref, value)
    local def = typedefs.struct_defs[type_ref.name]
    if not def then
        error("Unknown struct: " .. type_ref.name)
    end
    local result = {}
    for _, field in ipairs(def.fields) do
        result[field.name] = unwrap(field.type_ref, value[field.name])
    end
    return result
end

_unwrappers.vector = function(type_ref, value)
    local result = {}
    for i, elem in ipairs(value) do
        result[i] = unwrap(type_ref.type_ref, elem)
    end
    return result
end

-- ----------------------------------------------------------------------------
-- Public API
-- ----------------------------------------------------------------------------

local unwrapper = {}

unwrapper.unwrap = unwrap

return unwrapper
