-- struct_descriptor.lua
--
-- Descriptor constructors (D) for describing struct memory layouts.
--
-- Structs are described as an ordered array so that the offsetter can resolve
-- sizes in declaration order:
--
--   local structs = {
--       D.struct("Foo", { D.field("x", T.i32), D.pad(4), ... }),
--       D.struct("Bar", { D.field("foo", T.struct("Foo")), ... }),
--   }
--
-- Field offsets and struct sizes are not stored here; they are computed
-- by offsetter.offsets(structs).
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

D.struct = function(name, descriptors)
    return {name = name, descriptors = descriptors}
end

-- ----------------------------------------------------------------------------
-- validate(structs)
--
-- Checks that all type_refs are well-formed and that all cross-references
-- between structs are satisfied.  Operates on the raw descriptor arrays,
-- before any sizes or offsets are computed.
-- ----------------------------------------------------------------------------

local function validate(structs)
    local validate_type_ref = types.validate_type_ref

    -- Build a name->struct lookup for cross-reference checks.
    local struct_by_name = {}
    for _, s in ipairs(structs) do
        struct_by_name[s.name] = s
    end

    -- Pass 1: type_ref well-formedness on every field descriptor.
    for _, s in ipairs(structs) do
        for _, desc in ipairs(s.descriptors) do
            if desc.kind == "field" then
                validate_type_ref(desc.type_ref)
            end
        end
    end

    -- Pass 2: cross-reference checks now that all struct names are known.
    for _, s in ipairs(structs) do
        for _, desc in ipairs(s.descriptors) do
            if desc.kind == "field" then
                local tr = desc.type_ref

                if tr.kind == "struct" then
                    if not struct_by_name[tr.name] then
                        error("struct_descriptor.validate: struct '" .. s.name ..
                              "' field '" .. desc.name ..
                              "' references unknown struct '" .. tr.name .. "'")
                    end

                elseif tr.kind == "circular_list" then
                    local sub_name  = tr.type_ref.name
                    local sub_struct = struct_by_name[sub_name]
                    if not sub_struct then
                        error("struct_descriptor.validate: circular_list references " ..
                              "unknown struct '" .. sub_name .. "'")
                    end
                    -- Verify the 'next' field of the sub-struct.
                    local sub_descs = sub_struct.descriptors
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
