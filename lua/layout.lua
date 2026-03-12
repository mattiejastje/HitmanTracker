local T = {}
local _validators = {}

-- ----------------------------------------------------------------------------
-- Type references
-- ----------------------------------------------------------------------------

T.array = function(element_type_ref, count)
    return {kind = "array", type_ref = element_type_ref, count = count}
end
T.circular_list = function(element_type_ref)
    return {kind = "circular_list", type_ref = element_type_ref}
end
T.float = {kind = "float"}
T.i8 = {kind = "i8"}
T.i16 = {kind = "i16"}
T.i32 = {kind = "i32"}
T.i64 = {kind = "i64"}
T.optional_ptr = function(type_ref)
    return {kind = "ptr", type_ref = type_ref, optional = true, weak = true}
end
T.optional_ref = function(type_ref)
    return {kind = "ptr", type_ref = type_ref, optional = true, weak = false}
end
T.ptr = function(type_ref)
    return {kind = "ptr", type_ref = type_ref, optional = false, weak = true}
end
T.ref = function(type_ref)
    return {kind = "ptr", type_ref = type_ref, optional = false, weak = false}
end
T.string = function(max_length)
    return {kind = "string", max_length = max_length}
end
T.struct = function(name)
    return {kind = "struct", name = name}
end
T.vector = function(element_type_ref)
    return {kind = "vector", type_ref = element_type_ref}
end

-- ----------------------------------------------------------------------------
-- Validate
-- ----------------------------------------------------------------------------

local function _validate_type_ref(type_ref)
    if not type_ref.kind then
        error("type_ref has no kind")
    end
    local validator = _validators[type_ref.kind]
    if not validator then
        error("Unknown type kind: " .. type_ref.kind)
    end
    validator(type_ref)
end

_validators.array = function(type_ref)
    if not type_ref.type_ref then
        error("array has no type_ref")
    end
    if not type_ref.count then
        error("array has no count")
    end
    if type_ref.count < 1 then
        error("array count must be at least 1")
    end
    _validate_type_ref(type_ref.type_ref)
end

_validators.circular_list = function(type_ref)
    if not type_ref.type_ref then
        error("circular_list has no type_ref")
    end
    _validate_type_ref(type_ref.type_ref)
    local sub_name = type_ref.type_ref.name
    local def = layout.struct_defs[sub_name] -- always non-nil due to _validate_type_ref above
    for _, field in ipairs(def.fields) do
        if field.name == "next" then
            if field.type_ref.kind ~= "ptr" or not field.type_ref.weak then
                error(
                    "circular_list 'next' field in struct '" .. sub_name .. "' must be a ptr, got: " .. field.type_ref.kind
                )
            end
            local next_type = field.type_ref.type_ref
            if next_type.kind ~= "struct" or next_type.name ~= sub_name then
                error(
                    "circular_list 'next' field must be a ptr to struct '" .. sub_name .. "'"
                )
            end
            return -- found and validated
        end
    end
    error("circular_list 'next' field not found in struct '" .. sub_name .. "'")
end

_validators.float = function(type_ref) end
_validators.i8 = function(type_ref) end
_validators.i16 = function(type_ref) end
_validators.i32 = function(type_ref) end
_validators.i64 = function(type_ref) end

_validators.ptr = function(type_ref)
    if not type_ref.type_ref then
        error("ptr has no type_ref")
    end
    _validate_type_ref(type_ref.type_ref)
end

_validators.string = function(type_ref)
    if not type_ref.max_length then
        error("string has no max_length")
    end
    if type_ref.max_length < 1 then
        error("string max_length must be at least 1")
    end
end

_validators.struct = function(type_ref)
    if not layout.struct_defs[type_ref.name] then
        error("Unknown struct: " .. type_ref.name)
    end
end

_validators.vector = function(type_ref)
    if not type_ref.type_ref then
        error("vector has no type_ref")
    end
    _validate_type_ref(type_ref.type_ref)
end

-- ----------------------------------------------------------------------------
-- Public API
-- ----------------------------------------------------------------------------

local function validate_struct_defs()
    for name, def in pairs(layout.struct_defs) do
        if not def.fields then
            error("Struct '" .. name .. "' has no fields")
        end
        if not def.size then
            error("Struct '" .. name .. "' has no size")
        end
        for _, field in ipairs(def.fields) do
            if not field.name then
                error("Struct '" .. name .. "' has a field with no name")
            end
            if not field.offset then
                error("Struct '" .. name .. "' field '" .. field.name .. "' has no offset")
            end
            if not field.type_ref then
                error("Struct '" .. name .. "' field '" .. field.name .. "' has no type_ref")
            end
            _validate_type_ref(field.type_ref)
        end
    end
end

local layout = {}

layout.T = T
layout.struct_defs = {}
layout.validate_struct_defs = validate_struct_defs

return layout
