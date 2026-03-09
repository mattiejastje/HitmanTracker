local _T = {}
local _handlers = {}
local _ptr_size = getPointerSize()
local _structs = {}
local _validators = {}

_T.i8 = {category = "basic", name = "i8"}
_T.i16 = {category = "basic", name = "i16"}
_T.i32 = {category = "basic", name = "i32"}
_T.float = {category = "basic", name = "float"}
_T.struct = function(name)
    return {category = "struct", name = name}
end
_T.ptr = function(type_def)
    return {category = "ptr", type_def = type_def}
end
_T.weak_ptr = function(type_def)
    return {category = "weak_ptr", type_def = type_def}
end
_T.vector = function(element_type_def)
    return {category = "vector", type_def = element_type_def}
end
_T.circular_list = function(element_type_def, next_field)
    return {category = "circular_list", type_def = element_type_def, next_field = next_field}
end

local _basic_readers = {
    i8 = function(addr)
        return readByte(addr, true)
    end,
    i16 = function(addr)
        return readShortInteger(addr, true)
    end,
    i32 = function(addr)
        return readInteger(addr, true)
    end,
    float = readFloat
}

local _basic_sizes = {i8 = 1, i16 = 2, i32 = 4, float = 4}

local function _type_size(type_def)
    if type_def.category == "struct" then
        return _structs[type_def.name].size
    elseif type_def.category == "basic" then
        return _basic_sizes[type_def.name]
    elseif
        type_def.category == "ptr" or type_def.category == "weak_ptr" or type_def.category == "vector" or
            type_def.category == "circular_list"
     then
        -- vector stores two pointers (begin and end)
        return _ptr_size * (type_def.category == "vector" and 2 or 1)
    end
    error("Unknown type: " .. (type_def.name or type_def.category))
end

local function _read(addr, type_def)
    if addr == nil then
        return nil
    end
    local handler = _handlers[type_def.category]
    if not handler then
        error("Unknown type category: " .. type_def.category)
    end
    return handler(addr, type_def)
end

local function _validate(type_def)
    local validator = _validators[type_def.category]
    if not validator then
        error("Unknown type category: " .. type_def.category)
    end
    validator(type_def)
end

local function _print(type_def, value, indent, label)
    indent = indent or 0
    local pad = string.rep("  ", indent)
    local prefix = label and (pad .. label .. ": ") or pad

    if value == nil then
        print(prefix .. "nil")
        return
    end

    if type_def.category == "basic" then
        print(prefix .. tostring(value))
    elseif type_def.category == "weak_ptr" then
        print(prefix .. string.format("0x%X", value))
    elseif type_def.category == "ptr" then
        _print(type_def.type_def, value, indent, label)
    elseif type_def.category == "struct" then
        local def = _structs[value.__name]
        print(prefix .. value.__name)
        for _, field in ipairs(def.fields) do
            _print(field.type_def, value[field.name], indent + 1, field.name)
        end
    elseif type_def.category == "vector" then
        print(prefix .. "vector(" .. #value .. ")" .. (value.__error and " [" .. value.__error .. "]" or ""))
        for i, elem in ipairs(value) do
            _print(type_def.type_def, elem, indent + 1, "[" .. i - 1 .. "]")
        end
    elseif type_def.category == "circular_list" then
        print(prefix .. "circular_list(" .. #value .. ")" .. (value.__error and " [" .. value.__error .. "]" or ""))
        for i, elem in ipairs(value) do
            _print(type_def.type_def, elem, indent + 1, "[" .. i .. "]")
        end
    else
        print(prefix .. "?(" .. tostring(value) .. ")")
    end
end

local function _read_pointer(addr)
    local p = readPointer(addr)
    if p == 0 then
        return nil
    end -- for consistency
    return p
end

_handlers.basic = function(addr, type_def)
    local reader = _basic_readers[type_def.name]
    if not reader then
        error("Unknown basic type: " .. type_def.name)
    end
    return reader(addr)
end

_handlers.struct = function(addr, type_def)
    local name = type_def.name
    local def = _structs[name]
    if not def then
        error("Unknown struct: " .. name)
    end
    local obj = {__addr = addr, __name = name}
    for _, field in ipairs(def.fields) do
        obj[field.name] = _read(addr + field.offset, field.type_def)
    end
    return obj
end

_handlers.ptr = function(addr, type_def)
    local p = _read_pointer(addr)
    return _read(p, type_def.type_def)
end

_handlers.weak_ptr = function(addr, type_def)
    -- type_def is intentionally unused
    -- weak_ptr returns the raw address without dereferencing
    return _read_pointer(addr)
end

_handlers.vector = function(addr, type_def)
    local begin_ptr = _read_pointer(addr)
    local end_ptr = _read_pointer(addr + _ptr_size)
    local sub_type = type_def.type_def
    local obj = {__addr = addr, __begin_ptr = begin_ptr, __end_ptr = end_ptr, __type_def = sub_type}
    if begin_ptr == nil or end_ptr == nil then
        obj.__error = "Invalid vector pointers"
        return obj
    end
    if begin_ptr > end_ptr then
        obj.__error = "Vector begin pointer is past end pointer"
        return obj
    end
    local p = begin_ptr
    local size = _type_size(sub_type)
    while p < end_ptr do
        table.insert(obj, _read(p, sub_type))
        p = p + size
    end
    if p ~= end_ptr then
        obj.__error = "Vector pointer overflow"
    end
    return obj
end

_handlers.circular_list = function(addr, type_def)
    local sub_type = type_def.type_def
    local next_field = type_def.next_field
    local obj = {__sub_type = sub_type, __next_field = next_field}
    local head = readPointer(addr)
    if head == nil then
        obj.__error = "Invalid head pointer"
    elseif head ~= 0 then
        local p = head
        repeat
            local value = _read(p, sub_type)
            table.insert(obj, value)
            p = value[next_field]
            if p == nil then
                obj.__error = "Invalid next pointer at index " .. #obj
                return obj
            end
        until p == head
    end
    return obj
end

_validators.basic = function(type_def)
    if not _basic_sizes[type_def.name] then
        error("Unknown basic type: " .. type_def.name)
    end
end

_validators.struct = function(type_def)
    local name = type_def.name
    local def = _structs[name]
    if not def then
        error("Unknown struct: " .. name)
    end
    if not def.fields then
        error("Struct '" .. name .. "' has no fields")
    end
    if not def.size then
        error("Struct '" .. name .. "' has no size")
    end
    for _, field in ipairs(def.fields) do
        if not field.name then
            error("Struct '" .. name .. "' has a field with no name")
        end
        if not field.offset then
            error("Struct '" .. name .. "' field '" .. field.name .. "' has no offset")
        end
        if not field.type_def then
            error("Struct '" .. name .. "' field '" .. field.name .. "' has no type_def")
        end
        _validate(field.type_def)
    end
end

_validators.ptr = function(type_def)
    if not type_def.type_def then
        error("ptr has no type_def")
    end
    _validate(type_def.type_def)
end

_validators.weak_ptr = function(type_def)
    if not type_def.type_def then
        error("weak_ptr has no type_def")
    end
    _validate(type_def.type_def)
end

_validators.vector = function(type_def)
    if not type_def.type_def then
        error("vector has no type_def")
    end
    _validate(type_def.type_def)
end

_validators.circular_list = function(type_def)
    if not type_def.type_def then
        error("circular_list has no type_def")
    end
    if not type_def.next_field then
        error("circular_list has no next_field")
    end
    _validate(type_def.type_def)
    local sub_name = type_def.type_def.name
    local def = _structs[sub_name] -- always non-nil due to _validate above
    for _, field in ipairs(def.fields) do
        if field.name == type_def.next_field then
            if field.type_def.category ~= "weak_ptr" then
                error(
                    "circular_list next_field '" ..
                        type_def.next_field ..
                            "' in struct '" .. sub_name .. "' must be a weak_ptr, got: " .. field.type_def.category
                )
            end
            local next_type = field.type_def.type_def
            if next_type.category ~= "struct" or next_type.name ~= sub_name then
                error(
                    "circular_list next_field '" ..
                        type_def.next_field .. "' must be a weak_ptr to struct '" .. sub_name .. "'"
                )
            end
            return -- found and validated
        end
    end
    error("circular_list next_field '" .. type_def.next_field .. "' not found in struct '" .. sub_name .. "'")
end

local function _init(structs)
    _structs = structs
    for name, _ in pairs(_structs) do
        _validate({category = "struct", name = name})
    end
end

local memreader = {}

memreader.T = _T
memreader.init = _init
memreader.read = _read
memreader.print = _print

return memreader
