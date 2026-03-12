-- struct_descriptor.lua
--
-- Descriptor constructors (D) for describing struct memory layouts.
--
-- Each struct is described as a plain array of D.field / D.pad / D.offset
-- descriptors, collected into a table keyed by struct name:
--
--   struct_descriptors[name] = { D.field(...), D.pad(4), ... }
--
-- Field offsets and struct sizes are not stored here; they are computed
-- by struct_sizer.resolve and struct_builder.build respectively.
--
-- Type references (T) live in types.lua.

local types = require("types")

-- ----------------------------------------------------------------------------
-- Descriptors
-- ----------------------------------------------------------------------------

local D = {}

D.pad = function(n)
    return {kind = "pad", n = n}
end

D.offset = function(n)
    return {kind = "offset", n = n}
end

D.field = function(name, type_ref, opts)
    return {kind = "field", name = name, type_ref = type_ref, opts = opts}
end

-- ----------------------------------------------------------------------------
-- validate(struct_descriptors)
--
-- Checks that all type_refs are well-formed and that all cross-references
-- between structs are satisfied.  Operates on the raw descriptor arrays,
-- before any sizes or offsets are computed.
-- ----------------------------------------------------------------------------

local function validate(struct_descriptors)
    local validate_type_ref = types.validate_type_ref

    -- Pass 1: type_ref well-formedness on every field descriptor.
    for name, descriptors in pairs(struct_descriptors) do
        for _, desc in ipairs(descriptors) do
            if desc.kind == "field" then
                validate_type_ref(desc.type_ref)
            end
        end
    end

    -- Pass 2: cross-reference checks now that all struct names are known.
    for name, descriptors in pairs(struct_descriptors) do
        for _, desc in ipairs(descriptors) do
            if desc.kind == "field" then
                local tr = desc.type_ref

                if tr.kind == "struct" then
                    if not struct_descriptors[tr.name] then
                        error("struct_descriptor.validate: struct '" .. name ..
                              "' field '" .. desc.name ..
                              "' references unknown struct '" .. tr.name .. "'")
                    end

                elseif tr.kind == "circular_list" then
                    local sub_name  = tr.type_ref.name
                    local sub_descs = struct_descriptors[sub_name]
                    if not sub_descs then
                        error("struct_descriptor.validate: circular_list references " ..
                              "unknown struct '" .. sub_name .. "'")
                    end
                    local found = false
                    for _, sub_desc in ipairs(sub_descs) do
                        if sub_desc.kind == "field" and sub_desc.name == "next" then
                            if sub_desc.type_ref.kind ~= "ptr" or not sub_desc.type_ref.weak then
                                error(
                                    "struct_descriptor.validate: circular_list 'next' field " ..
                                    "in struct '" .. sub_name .. "' must be a ptr, got: " ..
                                    sub_desc.type_ref.kind
                                )
                            end
                            local next_type = sub_desc.type_ref.type_ref
                            if next_type.kind ~= "struct" or next_type.name ~= sub_name then
                                error(
                                    "struct_descriptor.validate: circular_list 'next' field " ..
                                    "must be a ptr to struct '" .. sub_name .. "'"
                                )
                            end
                            found = true
                            break
                        end
                    end
                    if not found then
                        error("struct_descriptor.validate: circular_list 'next' field " ..
                              "not found in struct '" .. sub_name .. "'")
                    end
                end
            end
        end
    end
end

-- ----------------------------------------------------------------------------
-- Public API
-- ----------------------------------------------------------------------------

local struct_descriptor = {}

struct_descriptor.D        = D
struct_descriptor.validate = validate

return struct_descriptor
