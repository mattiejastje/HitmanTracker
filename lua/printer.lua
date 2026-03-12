local typedefs = require("typedefs")

local _printers = {}

-- ----------------------------------------------------------------------------
-- Helpers
-- ----------------------------------------------------------------------------

local function print_line(addr, indent, label, value_str)
    local pad = string.rep("  ", indent)
    local addr_str = addr and string.format("[%08X]", addr) or "[--------]"
    local label_str = label and (label .. ": ") or ""
    print(addr_str .. " " .. pad .. label_str .. value_str)
end

-- ----------------------------------------------------------------------------
-- Dispatch
-- ----------------------------------------------------------------------------

local function print_rawval(type_ref, rawval, indent, label)
    indent = indent or 0

    if rawval == nil then
        print_line(nil, indent, label, "nil")
        return
    end

    local addr = rawval.addr

    if rawval.error then
        print_line(addr, indent, label, "ERROR: " .. rawval.error)
        if rawval.value == nil then
            return
        end
    elseif rawval.value == nil then
        print_line(addr, indent, label, "nil")
        return
    end

    local printer = _printers[type_ref.kind]
    if not printer then
        error("Unknown type kind: " .. type_ref.kind)
    end
    printer(type_ref, rawval.value, addr, indent, label)
end

-- ----------------------------------------------------------------------------
-- Printers
-- ----------------------------------------------------------------------------

_printers.array = function(type_ref, value, addr, indent, label)
    print_line(addr, indent, label, "array(" .. #value .. ")")
    for i, elem in ipairs(value) do
        print_rawval(type_ref.type_ref, elem, indent + 1, "[" .. i - 1 .. "]")
    end
end

_printers.circular_list = function(type_ref, value, addr, indent, label)
    print_line(addr, indent, label, "circular_list(" .. #value .. ")")
    for i, elem in ipairs(value) do
        print_rawval(type_ref.type_ref, elem, indent + 1, "[" .. i - 1 .. "]")
    end
end

_printers.float = function(type_ref, value, addr, indent, label)
    print_line(addr, indent, label, tostring(value))
end

_printers.i8 = function(type_ref, value, addr, indent, label)
    print_line(addr, indent, label, tostring(value))
end

_printers.i16 = function(type_ref, value, addr, indent, label)
    print_line(addr, indent, label, tostring(value))
end

_printers.i32 = function(type_ref, value, addr, indent, label)
    print_line(addr, indent, label, tostring(value))
end

_printers.i64 = function(type_ref, value, addr, indent, label)
    print_line(addr, indent, label, tostring(value))
end

_printers.ptr = function(type_ref, value, addr, indent, label)
    if type_ref.weak then
        print_line(addr, indent, label, string.format("0x%X", value))
    else
        print_line(addr, indent, label, "ptr")
        print_rawval(type_ref.type_ref, value, indent + 1, nil)
    end
end

_printers.string = function(type_ref, value, addr, indent, label)
    print_line(addr, indent, label, '"' .. value .. '"')
end

_printers.struct = function(type_ref, value, addr, indent, label)
    local def = typedefs.struct_defs[type_ref.name]
    if not def then
        error("Unknown struct: " .. type_ref.name)
    end
    print_line(addr, indent, label, type_ref.name)
    for _, field in ipairs(def.fields) do
        if field.print ~= false then
            print_rawval(field.type_ref, value[field.name], indent + 1, field.name)
        end
    end
end

_printers.vector = function(type_ref, value, addr, indent, label)
    print_line(addr, indent, label, "vector(" .. #value .. ")")
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