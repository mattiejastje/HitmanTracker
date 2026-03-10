local typedefs = require("typedefs")

local _printers = {}

-- ----------------------------------------------------------------------------
-- Dispatch
-- ----------------------------------------------------------------------------

local function print_value(type_ref, value, indent, label)
    indent = indent or 0
    local pad = string.rep("  ", indent)
    local prefix = label and (pad .. label .. ": ") or pad

    if value == nil then
        print(prefix .. "nil")
        return
    end

    local printer = _printers[type_ref.kind]
    if not printer then
        error("Unknown type kind: " .. type_ref.kind)
    end
    printer(type_ref, value, indent, prefix)
end

-- ----------------------------------------------------------------------------
-- Printers
-- ----------------------------------------------------------------------------

_printers.array = function(type_ref, value, indent, prefix)
    print(prefix .. "array(" .. #value .. ")")
    for i, elem in ipairs(value) do
        print_value(type_ref.type_ref, elem, indent + 1, "[" .. i - 1 .. "]")
    end
end

_printers.circular_list = function(type_ref, value, indent, prefix)
    print(prefix .. "circular_list(" .. #value .. ")")
    for i, elem in ipairs(value) do
        print_value(type_ref.type_ref, elem, indent + 1, "[" .. i .. "]")
    end
end

_printers.float = function(type_ref, value, indent, prefix)
    print(prefix .. tostring(value))
end

_printers.i8 = function(type_ref, value, indent, prefix)
    print(prefix .. tostring(value))
end

_printers.i16 = function(type_ref, value, indent, prefix)
    print(prefix .. tostring(value))
end

_printers.i32 = function(type_ref, value, indent, prefix)
    print(prefix .. tostring(value))
end

_printers.i64 = function(type_ref, value, indent, prefix)
    print(prefix .. tostring(value))
end

_printers.ptr = function(type_ref, value, indent, prefix)
    if type_ref.weak then
        print(prefix .. (value ~= nil and string.format("0x%X", value) or "nil"))
    else
        print(prefix .. "ptr")
        print_value(type_ref.type_ref, value, indent + 1, nil)
    end
end

_printers.string = function(type_ref, value, indent, prefix)
    print(prefix .. '"' .. value .. '"')
end

_printers.struct = function(type_ref, value, indent, prefix)
    local def = typedefs.struct_defs[type_ref.name]
    print(prefix .. type_ref.name)
    for _, field in ipairs(def.fields) do
        if field.print ~= false then
            print_value(field.type_ref, value[field.name], indent + 1, field.name)
        end
    end
end

_printers.vector = function(type_ref, value, indent, prefix)
    print(prefix .. "vector(" .. #value .. ")")
    for i, elem in ipairs(value) do
        print_value(type_ref.type_ref, elem, indent + 1, "[" .. i - 1 .. "]")
    end
end

-- ----------------------------------------------------------------------------
-- Public API
-- ----------------------------------------------------------------------------

local printer = {}

printer.print = print_value

return printer
