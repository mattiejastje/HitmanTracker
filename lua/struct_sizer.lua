-- struct_sizer.lua
--
-- struct_sizer.resolve(struct_descriptors) -> struct_info
--
--   Computes the size and field offsets of each struct.
--
--   struct_info[name] = {
--       size   = <number>,          -- total byte size of the struct
--       fields = {                  -- one entry per D.field descriptor, in order
--           {name=..., offset=...},
--           ...
--       },
--   }
--
--   Structs are resolved in dependency order: a struct containing an inline
--   (non-pointer) nested struct will always be resolved after it.
--
-- struct_sizer.new(struct_size_lookup) -> sizeof
--
--   Returns a sizeof(type_ref) function.
--   struct_size_lookup(name) must return the byte size of the named struct.

-- ----------------------------------------------------------------------------
-- make_sizeof(struct_lookup, context) -> sizeof
-- ----------------------------------------------------------------------------

local function make_sizeof(struct_lookup, context)
    local sizeof
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
-- inline_deps(descriptors) -> list of struct names embedded inline
--
-- Only D.field descriptors whose type_ref.kind == "struct" create a size
-- dependency; pointer-sized tags (ptr, vector, circular_list, array-of-struct)
-- do not.
-- ----------------------------------------------------------------------------

local function inline_deps(descriptors)
    local deps = {}
    for _, desc in ipairs(descriptors) do
        if desc.kind == "field" and desc.type_ref.kind == "struct" then
            deps[#deps + 1] = desc.type_ref.name
        end
    end
    return deps
end

-- ----------------------------------------------------------------------------
-- resolve(struct_descriptors) -> struct_info
-- ----------------------------------------------------------------------------

local function resolve(struct_descriptors)
    local struct_info = {}
    local resolving   = {}  -- cycle detection

    local function resolve_one(name)
        if struct_info[name] then return end
        if resolving[name] then
            error("struct_sizer.resolve: cyclic inline dependency on '" .. name .. "'")
        end
        resolving[name] = true

        local descriptors = struct_descriptors[name]
        if not descriptors then
            error("struct_sizer.resolve: unknown struct '" .. name .. "'")
        end

        -- Resolve inline dependencies before this struct.
        for _, dep in ipairs(inline_deps(descriptors)) do
            resolve_one(dep)
        end

        local sizeof = make_sizeof(
            function(n) return struct_info[n] and struct_info[n].size end,
            "struct_sizer.resolve"
        )

        local cursor = 0
        local fields = {}

        for _, desc in ipairs(descriptors) do
            if desc.kind == "pad" then
                cursor = cursor + desc.n
            elseif desc.kind == "offset" then
                if desc.n < cursor then
                    error(string.format(
                        "struct_sizer.resolve: D.offset(%d) in struct '%s' would move " ..
                        "cursor backwards (currently at %d)", desc.n, name, cursor))
                end
                cursor = desc.n
            elseif desc.kind == "field" then
                fields[#fields + 1] = {name = desc.name, offset = cursor}
                cursor = cursor + sizeof(desc.type_ref)
            else
                error("struct_sizer.resolve: unknown descriptor kind '" ..
                      tostring(desc.kind) .. "'")
            end
        end

        struct_info[name] = {size = cursor, fields = fields}
        resolving[name] = nil
    end

    for name in pairs(struct_descriptors) do
        resolve_one(name)
    end

    return struct_info
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
