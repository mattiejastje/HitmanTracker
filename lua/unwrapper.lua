local typedefs = require("typedefs")

local _unwrappers = {}

-- ----------------------------------------------------------------------------
-- Dispatch
-- ----------------------------------------------------------------------------

local function unwrap(type_ref, rawval)
    if rawval == nil or rawval.value == nil then
        local errors = rawval and rawval.error and {rawval.error} or {}
        return nil, errors
    end

    local unwrapper = _unwrappers[type_ref.kind]
    if not unwrapper then
        error("Unknown type kind: " .. type_ref.kind)
    end
    return unwrapper(type_ref, rawval.value)
end

-- ----------------------------------------------------------------------------
-- Helpers
-- ----------------------------------------------------------------------------

local function prefix_errors(errors, context)
    local result = {}
    for _, err in ipairs(errors) do
        table.insert(result, context .. ": " .. err)
    end
    return result
end

local function merge_errors(dest, src)
    for _, err in ipairs(src) do
        table.insert(dest, err)
    end
end

-- ----------------------------------------------------------------------------
-- Unwrappers
-- ----------------------------------------------------------------------------

_unwrappers.array = function(type_ref, value)
    local result = {}
    local errors = {}
    for i, elem in ipairs(value) do
        local v, errs = unwrap(type_ref.type_ref, elem)
        result[i] = v
        merge_errors(errors, prefix_errors(errs, "[" .. i - 1 .. "]"))
    end
    return result, errors
end

_unwrappers.circular_list = function(type_ref, value)
    local result = {}
    local errors = {}
    for i, elem in ipairs(value) do
        local v, errs = unwrap(type_ref.type_ref, elem)
        result[i] = v
        merge_errors(errors, prefix_errors(errs, "[" .. i - 1 .. "]"))
    end
    return result, errors
end

_unwrappers.float = function(type_ref, value)
    return value, {}
end

_unwrappers.i8 = function(type_ref, value)
    return value, {}
end

_unwrappers.i16 = function(type_ref, value)
    return value, {}
end

_unwrappers.i32 = function(type_ref, value)
    return value, {}
end

_unwrappers.i64 = function(type_ref, value)
    return value, {}
end

_unwrappers.ptr = function(type_ref, value)
    if type_ref.weak then
        return value, {}
    end
    return unwrap(type_ref.type_ref, value)
end

_unwrappers.string = function(type_ref, value)
    return value, {}
end

_unwrappers.struct = function(type_ref, value)
    local def = typedefs.struct_defs[type_ref.name]
    if not def then
        error("Unknown struct: " .. type_ref.name)
    end
    local result = {}
    local errors = {}
    for _, field in ipairs(def.fields) do
        local v, errs = unwrap(field.type_ref, value[field.name])
        result[field.name] = v
        merge_errors(errors, prefix_errors(errs, field.name))
    end
    return result, errors
end

_unwrappers.vector = function(type_ref, value)
    local result = {}
    local errors = {}
    for i, elem in ipairs(value) do
        local v, errs = unwrap(type_ref.type_ref, elem)
        result[i] = v
        merge_errors(errors, prefix_errors(errs, "[" .. i - 1 .. "]"))
    end
    return result, errors
end

-- ----------------------------------------------------------------------------
-- Public API
-- ----------------------------------------------------------------------------

local unwrapper = {}

-- Returns value, errors. errors is a flat list of context-prefixed strings.
unwrapper.unwrap = unwrap

return unwrapper
