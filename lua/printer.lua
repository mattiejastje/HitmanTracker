local typedefs = require("typedefs")

local _printers = {}

-- ----------------------------------------------------------------------------
-- Dispatch
-- ----------------------------------------------------------------------------

local function print_rawval(type_ref, rawval, indent, label)
    indent = indent or 0
    local pad = string.rep("  ", indent)
    local prefix = label and (pad .. label .. ": ") or pad

    if rawval == nil then
        print(prefix .. "nil")
        return
    end

    local addr_str = rawval.addr and string.format("0x%X", rawval.addr) or "nil"

    if rawval.error then
        print(prefix .. "ERROR @ " .. addr_str .. ": " .. rawval.error)
        return
    end

    if rawval.value == nil then
        print(prefix .. "nil @ " .. addr_str)
        return
    end

    local printer = _printers[type_ref.kind]
    if not printer then
        error("Unknown type kind: " .. type_ref.kind)
    end
    printer(type_ref, rawval.value, rawval.addr, indent, prefix)
end

-- ----------------------------------------------------------------------------
-- Printers
-- ----------------------------------------------------------------------------

_printers.array = function(type_ref, value, addr, indent, prefix)
    print(prefix .. "array(" .. #value .. ") @ " .. string.format("0x%X", addr))
    for i, elem in ipairs(value) do
        print_rawval(type_ref.type_ref, elem, indent + 1, "[" .. i - 1 .. "]")
    end
end

_printers.circular_list = function(type_ref, value, addr, indent, prefix)
    print(prefix .. "circular_list(" .. #value .. ") @ " .. string.format("0x%X", addr))
    for i, elem in ipairs(value) do
        print_rawval(type_ref.type_ref, elem, indent + 1, "[" .. i - 1 .. "]")
    end
end

_printers.float = function(type_ref, value, addr, indent, prefix)
    print(prefix .. tostring(value) .. " @ " .. string.format("0x%X", addr))
end

_printers.i8 = function(type_ref, value, addr, indent, prefix)
    print(prefix .. tostring(value) .. " @ " .. string.format("0x%X", addr))
end

_printers.i16 = function(type_ref, value, addr, indent, prefix)
    print(prefix .. tostring(value) .. " @ " .. string.format("0x%X", addr))
end

_printers.i32 = function(type_ref, value, addr, indent, prefix)
    print(prefix .. tostring(value) .. " @ " .. string.format("0x%X", addr))
end

_printers.i64 = function(type_ref, value, addr, indent, prefix)
    print(prefix .. tostring(value) .. " @ " .. string.format("0x%X", addr))
end

_printers.ptr = function(type_ref, value, addr, indent, prefix)
    if type_ref.weak then
        print(prefix .. string.format("0x%X", value) .. " @ " .. string.format("0x%X", addr))
    else
        print(prefix .. "ptr @ " .. string.format("0x%X", addr))
        print_rawval(type_ref.type_ref, value, indent + 1, nil)
    end
end

_printers.string = function(type_ref, value, addr, indent, prefix)
    print(prefix .. '"' .. value .. '" @ ' .. string.format("0x%X", addr))
end

_printers.struct = function(type_ref, value, addr, indent, prefix)
    local def = typedefs.struct_defs[type_ref.name]
    if not def then
        error("Unknown struct: " .. type_ref.name)
    end
    print(prefix .. type_ref.name .. " @ " .. string.format("0x%X", addr))
    for _, field in ipairs(def.fields) do
        if field.print ~= false then
            print_rawval(field.type_ref, value[field.name], indent + 1, field.name)
        end
    end
end

_printers.vector = function(type_ref, value, addr, indent, prefix)
    print(prefix .. "vector(" .. #value .. ") @ " .. string.format("0x%X", addr))
    for i, elem in ipairs(value) do
        print_rawval(type_ref.type_ref, elem, indent + 1, "[" .. i - 1 .. "]")
    end
end

-- ----------------------------------------------------------------------------
-- Public API
-- ----------------------------------------------------------------------------

local printer = {}

printer.print = print_rawval

return printer
