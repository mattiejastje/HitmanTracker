-- types.lua
--
-- Type reference constructors (T) and type_ref validation.
--
-- A type_ref is a plain table describing the type of a value in the target
-- process.  These are consumed by sizer.lua, reader.lua, printer.lua, and
-- unwrapper.lua.  They are produced here and used as arguments to the
-- layout descriptors in layout.lua.

local T = {}
local _validators = {}

-- ----------------------------------------------------------------------------
-- Type reference constructors
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
-- Validation
-- ----------------------------------------------------------------------------

local function validate_type_ref(type_ref)
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
    validate_type_ref(type_ref.type_ref)
end

_validators.circular_list = function(type_ref)
    if not type_ref.type_ref then
        error("circular_list has no type_ref")
    end
    validate_type_ref(type_ref.type_ref)
    -- Structural check (next field) is deferred to layout.validate_struct_defs,
    -- where struct_defs is guaranteed to be populated.
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
    validate_type_ref(type_ref.type_ref)
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
    -- Existence check is deferred to struct_descriptor.validate.
    if not type_ref.name then
        error("struct type_ref has no name")
    end
end

_validators.vector = function(type_ref)
    if not type_ref.type_ref then
        error("vector has no type_ref")
    end
    validate_type_ref(type_ref.type_ref)
end

-- ----------------------------------------------------------------------------
-- Public API
-- ----------------------------------------------------------------------------

local types = {}

types.T = T
types.validate_type_ref = validate_type_ref

return types
