-- struct_builder.lua
--
-- struct_builder.build(struct_descriptors, struct_size) -> struct
--
--   Walks each struct's descriptor array with a cursor, assigning each field
--   its offset.  struct_size (produced by struct_sizer.resolve) provides the
--   byte size of each struct, needed to advance the cursor past inline nested
--   struct fields.
--
--   struct[name] is the fully resolved form consumed by reader, printer, and
--   unwrapper:
--
--     struct[name] = {
--         fields = {
--             {name="...", type_ref=T..., offset=0,  opts=...},
--             {name="...", type_ref=T..., offset=14, opts=...},
--         },
--         size = ...,
--     }

local struct_sizer = require("struct_sizer")

-- ----------------------------------------------------------------------------
-- build(struct_descriptors, struct_size) -> struct
-- ----------------------------------------------------------------------------

local function build(struct_descriptors, struct_size)
    local sizeof = struct_sizer.new(function(name) return struct_size[name] end)
    local struct = {}

    local desc_handlers = {
        pad    = function(desc, cursor) return cursor + desc.n end,
        offset = function(desc, cursor, name)
            if desc.n < cursor then
                error(string.format(
                    "struct_builder.build: D.offset(%d) in struct '%s' would move cursor " ..
                    "backwards (currently at %d)", desc.n, name, cursor))
            end
            return desc.n
        end,
        field  = function(desc, cursor, fields)
            fields[#fields + 1] = {
                name     = desc.name,
                type_ref = desc.type_ref,
                offset   = cursor,
                opts     = desc.opts,
            }
            return cursor + sizeof(desc.type_ref)
        end,
    }

    for name, descriptors in pairs(struct_descriptors) do
        local fields = {}
        local cursor = 0
        for _, desc in ipairs(descriptors) do
            local handler = desc_handlers[desc.kind]
            if not handler then
                error("struct_builder.build: unknown descriptor kind '" ..
                      tostring(desc.kind) .. "'")
            end
            cursor = handler(desc, cursor, name, fields)
        end
        struct[name] = {fields = fields, size = struct_size[name]}
    end

    return struct
end

-- ----------------------------------------------------------------------------
-- Public API
-- ----------------------------------------------------------------------------

local struct_builder = {}

struct_builder.build = build

return struct_builder
