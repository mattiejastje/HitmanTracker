M = {}

local d = require("mempeep.descriptors")
local read = require("mempeep.read")

local SmallString = d.Struct("SmallString", {
    d.Field(d.Ref(d.ZString(0x100)), "text"),
    d.Skip(0x7C),  -- inline buffer for strings <= 0x7C chars
})

M.LevelControl = d.Struct("LevelControl", {
    d.Seek(0x154),
    d.Field(SmallString, "unk_scene_name"),  -- "C0-2\C0-2__MAIN"
    d.Seek(0x208),
    d.Field(d.Int32, "headshots"),
    d.Field(d.Int32, "enemies_wounded"),
    d.Field(d.Int32, "enemies_killed"),
    d.Field(d.Int32, "innocents_wounded"),
    d.Field(d.Int32, "innocents_killed"),
    d.Field(d.Int32, "alerts"),
    d.Field(d.Int32, "close_encounters"),
})

local EntityManager = d.Struct("EntityManager", {
    -- entity handle = (version << 18) | (index & 0x3FFFF)
    d.Seek(0x24),
    d.Field(d.Ref(d.RemoteAddr(d.Array(d.Int32, 0x40000))), "versions"),  -- (version << 18) of each object
    d.Field(d.Ref(d.RemoteAddr(d.Array(d.RawAddr(), 0x40000))), "entities"),  -- entities themselves
})

local PropertyType = d.Struct("PropertyType", {
    d.Field(d.UInt32, "fourcc"),  -- space padded fourcc code (always 4 chars, reverse order)
    d.Field(d.Int32, "fourcc_length"),  -- length of code without padding
    d.Field(d.Int32, "unk_id"),  -- unique number, maybe some id or index?
    d.Field(d.Int32, "size"),  -- size of the data in bytes
    d.Field(d.Int32, "unk_elem_type"),  -- maybe element type: 1 = bytes, 2 = characters, 4 = ints, 8 = floats, ...?
})

--- map PropertyType.fourcc to struct
M.property_struct = {
    -- "r": entity manager handle (version << 18) | (index & 0x3FFFF)
    [0x72202020] = d.UInt32,
    -- "gref": global reference handle
    -- a gref is an offset relative to gref_manager.pool.base
    -- the pool itself is presumably relocatable
    -- tagged with 0x40000000 to mark as gref (as opposed to regular pointer)
    -- 0x40000000 | (addr - gref_manager.pool.base) & 0x3FFFFFFF)
    [0x67726566] = d.UInt32,
    -- "l": integer
    [0x6c202020] = d.Int32,
    -- "f": float
    [0x66202020] = d.Float,
    -- "b": bool
    [0x62202020] = d.Int32,
}

M.Property = d.Struct("Property", {
    d.Field(d.Int32, "key_length"),  -- including terminating null
    d.Field(d.Ref(PropertyType), "type"),  -- type of data
    d.Field(d.Int32, "size"),   -- size of the data in bytes
    d.Field(d.ZString(0x40), "key"),  -- the key string
    -- data follows immediately after the key string
    -- offset is not static, need to calculate at runtime: 0x0C + key_length
    -- number of elements is size / type.size
})

local PropertyBlock = d.Struct("PropertyBlock", {
    d.Field(d.RawAddr(), "prev_block"),  -- address of previous block (null if first)
    d.Field(d.RawAddr(), "next_block"),  -- address of next block (null if last)
    d.Field(d.Int32, "num_properties"),  -- number of properties in this block
    d.Field(d.Int32, "tombstone_marker"),  -- property pointers beyond num_properties may be equal to this value (though not always)
    d.Field(d.Array(d.RawAddr(), 0x20), "properties")  --- pointer to each property, size in bytes is max_num_properties_per_block * property_size * 4
})

local SharedComContainer = d.Struct("SharedComContainer", {
    d.Field(d.RawAddr(), "vtable"),
    d.Field(d.List(PropertyBlock, "next_block", d.list_kind.NULL_TERMINATED, 0x1000), "blocks"),
    d.Field(d.RawAddr(PropertyBlock), "last_block"),
    d.Field(d.Int32, "unk_flags"),  -- unknown, always 0x20, 0x80000000 is cleared on access
    d.Field(d.Bounded(d.Int32, 0x20, 0x20), "max_num_properties_per_block"),  -- maximum number of properties per block (always 0x20)
    d.Field(d.Int32, "num_properties_total"),  -- total number of properties across all blocks
    d.Field(d.Bounded(d.Int32, 1, 1), "property_size"), -- always 1 (size of each property reference block in units of 4 bytes)
})

local SharedCom = d.Struct("SharedCom", {
    d.Field(d.RawAddr(), "vtable"),
    d.Seek(0x4008),
    d.Field(SharedComContainer, "container"),
})

local GRefManagerPool = d.Struct("GRefManagerPool", {
    d.Skip(0x4),
    -- base address of memory pool
    -- size is gref_manager.pool_size (in bytes)
    d.Field(d.RawAddr(), "base")
})

local GRefManager = d.Struct("GRefManager", {
    d.Seek(0x14),
    d.Field(d.Ref(GRefManagerPool), "pool"),  -- relocateble objects referenced by gref
    d.Seek(0x24),
    d.Field(d.Int32, "unk_24_flag"),  -- 1 if allocated?
    d.Field(d.Int32, "pool_size")
})

local SceneManager = d.Struct("SceneManager", {
    d.Skip(0x4),
    d.Field(d.Ref(GRefManager), "gref_manager"),  -- holds relocatable scene memory pool
    d.Seek(0xBB7),
    d.Field(SmallString, "scene_name"),
    d.Seek(0x1C4B),
    d.Field(SharedCom, "shared_com"),  -- holds scene properties
})

local Engine = d.Struct("Engine", {
    d.Seek(0x98),
    d.Field(d.Ref(SceneManager), "scene_manager"),
})

M.Game = d.Struct("Game", {
    d.Seek(0x2A6C50),
    d.Field(d.Ref(EntityManager), "entity_manager"),
    d.Seek(0x2A6C5C),
    d.Field(d.Ref(Engine), "engine"),
    d.Seek(0x28AA18),
    d.Field(d.ZString(0x40), "lethed"),  -- literal string constant
    d.Seek(0x2B3418),
    d.Field(d.ZString(0x0E), "current_mission_name"),  -- only during stats screen
    d.Seek(0x2B3420),
    d.Field(d.Int32, "shots_fired"),  -- only during stats screen
    d.Field(d.Int32, "close_encounters"),  -- only during stats screen
    d.Field(d.Int32, "headshots"),  -- only during stats screen
    d.Field(d.Int32, "alerts"),  -- only during stats screen
    d.Field(d.Int32, "enemies_killed"),  -- only during stats screen
    d.Field(d.Int32, "enemies_wounded"),  -- only during stats screen
    d.Field(d.Int32, "innocents_killed"),  -- only during stats screen
    d.Field(d.Int32, "innocents_wounded"),  -- only during stats screen
    d.Field(d.Int32, "stealth"),  -- only during stats screen
    d.Field(d.Int32, "aggression"),  -- only during stats screen
})

--- Get all valid property addresses from the shared_com container.
M.get_properties_addrs = function(shared_com_container_blocks)
    local addrs = {}
    for _, block in ipairs(shared_com_container_blocks) do
        for i = 1,block.num_properties,1 do
            addrs[#addrs + 1] = block.properties[i]
        end
    end
    return addrs
end

--- Read all properties and their data at the given addresses.
M.read_properties = function(addrs, reader, tracer)
    local ok = true
    local properties = {}
    for _, addr in ipairs(addrs) do
        local remote_property = d.remote_value(M.Property, addr)
        local property, ok = read.read(remote_property, reader, tracer)
        if not ok then
            return properties, false
        end
        struc = M.property_struct[property.type.fourcc]
        if not struc then
            error(string.format("unknown type fourcc: 0x%x", property.type.fourcc))
        end
        local data = {}
        for offset = 0,property.size-1,property.type.size do
            local remote_data = d.remote_value(struc, addr + 0x0C + property.key_length + offset)
            data[#data+1], ok = read.read(remote_data, reader, tracer)
            if not ok then
                return properties, false
            end
        end
        properties[property.key] = {
            type_fourcc = property.type.fourcc,
            data = data,
        }
    end
    return properties, true
end

M.mission_scene_names = {
    "SCENES\\C0-1\\C0-1__MAIN.gms",  -- sanctuary
    "SCENES\\C1-1\\C1-1__MAIN.gms",  -- anathema
    "SCENES\\C2-1\\C2-1__MAIN.gms",  -- stakeout
    "SCENES\\C2-2\\C2-2__MAIN.gms",  -- kirov
    "SCENES\\C2-3\\C2-3__MAIN.gms",  -- tubeway
    "SCENES\\C2-4\\C2-4__MAIN.gms",  -- invitation
    "SCENES\\C3-1\\C3-1__MAIN.gms",  -- tracking
    "SCENES\\C3-2a\\C3-2a__MAIN.gms",  -- hidden valley
    "SCENES\\C3-2b\\C3-2b__MAIN.gms",  -- gates
    "SCENES\\C3-3\\C3-3__MAIN.gms",  -- showdown
    "SCENES\\C4-1\\C4-1__MAIN.gms",  -- basement
    "SCENES\\C4-2\\C4-2__MAIN.gms",  -- graveyard
    "SCENES\\C4-3\\C4-3__MAIN.gms",  -- jacuzzi
    "SCENES\\C5-1\\C5-1__MAIN.gms",  -- bazaar
    "SCENES\\C5-2\\C5-2__MAIN.gms",  -- motorcade
    "SCENES\\C5-3\\C5-3__MAIN.gms",  -- tunnel rat
    "SCENES\\C6-1\\C6-1__MAIN.gms",  -- temple city
    "SCENES\\C6-2\\C6-2__MAIN.gms",  -- hannelore
    "SCENES\\C6-3\\C6-3__MAIN.gms",  -- hospitality
    "SCENES\\C7-1\\C7-1__MAIN.gms",  -- revisited
    "SCENES\\C8-1\\C8-1__MAIN.gms",  -- finale
}

-- The level control code is the index of the level control entity.
-- It appears to be deterministic for each mission.
-- Possibly quite fragile, but seems to work consistently.
-- We store it here without the 0x40000 tag so we can use it directly as an index.
M.level_control_code = {
    0x205,
    0x20E, 0x2C9, 0x228, 0x4E, 0x2E2, 0x2EE, 0x2D2, 0x33A, 0x4DB, 0x2B4,
    0x3D4, 0x235, 0x27B, 0x100, 0x27B, 0x191, 0x2C2, 0x25B, 0x2C0, 0x2
}

--- Return mission as index between 1 and 21 (or nil if not a mission name).
M.get_mission_index = function(scene_name)
    for i, v in ipairs(M.mission_scene_names) do
        if v == scene_name then
            return i
        end
    end
    return nil
end

M.resolve_gref = function(base, handle)
    if (handle & 0x40000000) == 0 then
        assert(handle == 0)
        return 0
    end
    offset = handle & 0x3FFFFFFF
    return base + offset
end

M.resolve_entity = function(versions, entities, handle)
    local version = handle >> 18
    assert(version >= 1)
    local index = handle & 0x3FFFF
    local actual_version = versions[index + 1] >> 18
    assert(actual_version == version)
    return entities[index + 1]
end

M.get_property_entity = function(versions, entities, properties, name)
    local property = properties[name]
    assert(property)
    local handle = property.data[1]
    assert(handle)
    assert(type(handle) == "number")
    assert(math.type(handle) == "integer")
    return M.resolve_entity(versions, entities, handle)
end

--- Get the level control address similar to how the game does it.
M.get_level_control_addr_1 = function(versions, entities, properties)
    return M.get_property_entity(versions, entities, properties, "LevelControlCode")
end

--- Get the level control address using lookup table.
-- mission_index is number from 1 to 21 (1 for training, 2 for first real mission)
M.get_level_control_addr_2 = function(versions, entities, mission_index)
    local index = M.level_control_code[mission_index]
    assert(versions[index + 1] == 0x40000)
    return entities[index + 1]
end

--- Calculate aggression from player statistics.
-- Can be at most 2 for silent assassin rating.
M.measure_aggression = function(level_control, shots_fired)
    -- raw measure of aggression (non-negative)
    local value = (
        3 * level_control.innocents_wounded + 6 * level_control.innocents_killed
        + level_control.enemies_wounded + 3 * level_control.enemies_killed
        + 2 * data.shots_fired
        + level_control.headshots + level_control.close_encounters
    )
    -- convert to [0,100] scale
    -- 0.5 * value for small values, ramping off at 100
    local aggression = math.floor(100 * math.tanh(0.005 * value))
    return aggression
end

--- Calculate stealth from player statistics.
-- Must be above 85 for silent assassin rating.
M.measure_stealth = function(level_control)
    -- raw measure of stealth (non-negative)
    local value = level_control.alerts + level_control.close_encounters
    -- convert to [0,100] scale using power law
    return math.floor(100 * (0.9 ^ value))
end

return M
