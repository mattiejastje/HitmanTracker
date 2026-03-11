local typedefs = require("typedefs")

local _sizers = {}

-- ----------------------------------------------------------------------------
-- Dispatch
-- ----------------------------------------------------------------------------

local function sizeof(type_ref)
    local sizer = _sizers[type_ref.kind]
    if not sizer then
        error("Unknown type kind: " .. type_ref.kind)
    end
    return sizer(type_ref)
end

-- ----------------------------------------------------------------------------
-- Sizers
-- ----------------------------------------------------------------------------

_sizers.array = function(type_ref)
    return sizeof(type_ref.type_ref) * type_ref.count
end

_sizers.circular_list = function(type_ref)
    return getPointerSize()
end

_sizers.float = function(type_ref)
    return 4
end

_sizers.i8 = function(type_ref)
    return 1
end

_sizers.i16 = function(type_ref)
    return 2
end

_sizers.i32 = function(type_ref)
    return 4
end

_sizers.i64 = function(type_ref)
    return 8
end

_sizers.ptr = function(type_ref)
    return getPointerSize()
end

_sizers.string = function(type_ref)
    return type_ref.max_length
end

_sizers.struct = function(type_ref)
    local def = typedefs.struct_defs[type_ref.name]
    if not def then
        error("Unknown struct: " .. type_ref.name)
    end
    return def.size
end

_sizers.vector = function(type_ref)
    -- vector stores two pointers (begin and end)
    return getPointerSize() * 2
end

-- ----------------------------------------------------------------------------
-- Public API
-- ----------------------------------------------------------------------------

local sizer = {}

sizer.sizeof = sizeof

return sizer
