-- struct_sizer.lua
--
-- struct_sizer.resolve(struct_descriptors) -> struct_size
--
--   Computes the size of each struct by summing its field sizes.  Structs
--   containing inline (non-pointer) nested structs must appear in
--   struct_descriptors before the structs that embed them.
--
--   struct_size[name] is a plain number: the byte size of that struct.
--
-- struct_sizer.new(struct_size_lookup) -> sizeof
--
--   Returns a sizeof(type_ref) function.
--   struct_size_lookup(name) must return the byte size of the named struct.

-- ----------------------------------------------------------------------------
-- make_sizeof(struct_lookup, context) -> sizeof
--
-- Builds a sizeof(type_ref) function.  struct_lookup(name) must return the
-- byte size of a named struct as a number.
-- ----------------------------------------------------------------------------

local function make_sizeof(struct_lookup, context)
    local sizeof  -- forward declaration for array recursion
    local _sizers = {
        i8            = function(tr) return 1 end,
        i16           = function(tr) return 2 end,
        i32           = function(tr) return 4 end,
        float         = function(tr) return 4 end,
        i64           = function(tr) return 8 end,
        string        = function(tr) return tr.max_length end,
        ptr           = function(tr) return getPointerSize() end,
        circular_list = function(tr) return getPointerSize() end,
        vector        = function(tr) return getPointerSize() * 2 end,
        array         = function(tr) return sizeof(tr.type_ref) * tr.count end,
        struct        = function(tr)
            local size = struct_lookup(tr.name)
            if not size then
                error(context .. ": unknown struct '" .. tr.name .. "'")
            end
            return size
        end,
    }
    sizeof = function(type_ref)
        local s = _sizers[type_ref.kind]
        if not s then
            error(context .. ": unknown type kind '" .. type_ref.kind .. "'")
        end
        return s(type_ref)
    end
    return sizeof
end

-- ----------------------------------------------------------------------------
-- resolve(struct_descriptors) -> struct_size
-- ----------------------------------------------------------------------------

local function resolve(struct_descriptors)
    local struct_size = {}
    local sizeof = make_sizeof(function(name) return struct_size[name] end,
                               "struct_sizer.resolve")

    local desc_handlers = {
        pad    = function(desc, cursor, name) return cursor + desc.n end,
        offset = function(desc, cursor, name)
            if desc.n < cursor then
                error(string.format(
                    "struct_sizer.resolve: D.offset(%d) in struct '%s' would move cursor " ..
                    "backwards (currently at %d)", desc.n, name, cursor))
            end
            return desc.n
        end,
        field  = function(desc, cursor, name) return cursor + sizeof(desc.type_ref) end,
    }

    for name, descriptors in pairs(struct_descriptors) do
        local cursor = 0
        for _, desc in ipairs(descriptors) do
            local handler = desc_handlers[desc.kind]
            if not handler then
                error("struct_sizer.resolve: unknown descriptor kind '" ..
                      tostring(desc.kind) .. "'")
            end
            cursor = handler(desc, cursor, name)
        end
        struct_size[name] = cursor
    end

    return struct_size
end

-- ----------------------------------------------------------------------------
-- new(struct_size_lookup) -> sizeof
--
-- struct_size_lookup(name) must return the byte size of the named struct.
-- ----------------------------------------------------------------------------

local function new(struct_size_lookup)
    return make_sizeof(struct_size_lookup, "sizeof")
end

-- ----------------------------------------------------------------------------
-- Public API
-- ----------------------------------------------------------------------------

local struct_sizer = {}

struct_sizer.resolve = resolve
struct_sizer.new     = new

return struct_sizer
