-- unwrapper.lua
--
-- Provides unwrapper.new(struct) -> unwrap, where unwrap(type_ref, rawval)
-- converts a raw value tree produced by reader into plain Lua values,
-- surfacing errors as a flat list of context-prefixed strings.
--
-- struct is the fully resolved struct table produced by struct_builder.build().

local function new(struct)
    local unwrap  -- forward declaration for mutual recursion

    -- ----------------------------------------------------------------------------
    -- Helpers
    -- ----------------------------------------------------------------------------

    local function prefix_errors(errors, context)
        local result = {}
        for _, err in ipairs(errors) do
            result[#result + 1] = context .. ": " .. err
        end
        return result
    end

    local function merge_errors(dest, src)
        for _, err in ipairs(src) do
            dest[#dest + 1] = err
        end
    end

    -- ----------------------------------------------------------------------------
    -- Unwrappers
    -- ----------------------------------------------------------------------------

    local _unwrappers = {}

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

    _unwrappers.float  = function(type_ref, value) return value, {} end
    _unwrappers.i8     = function(type_ref, value) return value, {} end
    _unwrappers.i16    = function(type_ref, value) return value, {} end
    _unwrappers.i32    = function(type_ref, value) return value, {} end
    _unwrappers.i64    = function(type_ref, value) return value, {} end
    _unwrappers.string = function(type_ref, value) return value, {} end

    _unwrappers.ptr = function(type_ref, value)
        if type_ref.weak then
            return value, {}
        end
        return unwrap(type_ref.type_ref, value)
    end

    _unwrappers.struct = function(type_ref, value)
        local def = struct[type_ref.name]
        if not def then
            error("unwrap: unknown struct '" .. type_ref.name .. "'")
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
    -- Dispatch
    -- ----------------------------------------------------------------------------

    unwrap = function(type_ref, rawval)
        if rawval == nil or rawval.value == nil then
            local errors = rawval and rawval.error and {rawval.error} or {}
            return nil, errors
        end
        local u = _unwrappers[type_ref.kind]
        if not u then
            error("unwrap: unknown type kind: " .. type_ref.kind)
        end
        return u(type_ref, rawval.value)
    end

    return unwrap
end

-- ----------------------------------------------------------------------------
-- Public API
-- ----------------------------------------------------------------------------

local unwrapper = {}

-- Returns value, errors. errors is a flat list of context-prefixed strings.
unwrapper.new = new

return unwrapper
