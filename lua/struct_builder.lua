-- struct_builder.lua
--
-- struct_builder.build(struct_descriptors, struct_info) -> struct
--
--   Assembles the fully resolved struct table consumed by reader, printer,
--   and unwrapper.  All cursor arithmetic is done by struct_sizer.resolve;
--   this module only zips the pre-computed offsets with type_refs and opts
--   from the original descriptors.
--
--   struct_info is the table returned by struct_sizer.resolve:
--
--     struct_info[name] = {
--         size   = <number>,
--         fields = { {name=..., offset=...}, ... },
--     }
--
--   Output:
--
--     struct[name] = {
--         fields = {
--             {name="...", type_ref=T..., offset=0,  opts=...},
--             {name="...", type_ref=T..., offset=14, opts=...},
--         },
--         size = ...,
--     }

local function build(struct_descriptors, struct_info)
    local struct = {}

    for name, info in pairs(struct_info) do
        local descriptors = struct_descriptors[name]

        -- Index field descriptors by name for O(1) lookup.
        local field_desc_by_name = {}
        for _, desc in ipairs(descriptors) do
            if desc.kind == "field" then
                field_desc_by_name[desc.name] = desc
            end
        end

        -- Zip pre-computed offsets with type_refs and opts.
        -- info.fields is already in declaration order.
        local fields = {}
        for _, f in ipairs(info.fields) do
            local desc = field_desc_by_name[f.name]
            fields[#fields + 1] = {
                name     = f.name,
                type_ref = desc.type_ref,
                offset   = f.offset,
                opts     = desc.opts,
            }
        end

        struct[name] = {fields = fields, size = info.size}
    end

    return struct
end

-- ----------------------------------------------------------------------------
-- Public API
-- ----------------------------------------------------------------------------

local struct_builder = {}

struct_builder.build = build

return struct_builder
