M = {}

local d = require("mempeep.descriptors")
local read = require("mempeep.read")

M.SmallString = d.Struct("SmallString", {
    d.Field(d.Ref(d.ZString(0x100)), "text"),
    d.Skip(0x7C),  -- inline buffer for strings <= 0x7C chars
})

local Vector3 = d.Array(d.Float, 3)
local Matrix33 = d.Array(d.Array(d.Float, 3), 3)

M.LevelControl = d.Struct("LevelControl", {
    d.Skip(0x208),
    d.Field(d.Int32, "headshots"),
    d.Field(d.Int32, "enemies_wounded"),
    d.Field(d.Int32, "enemies_killed"),
    d.Field(d.Int32, "innocents_wounded"),
    d.Field(d.Int32, "innocents_killed"),
    d.Field(d.Int32, "alerts"),
    d.Field(d.Int32, "close_encounters"),
})

M.EntityManager = d.Struct("EntityManager", {
    -- entity handle = (version << 18) | (index & 0x3FFFF)
    d.Seek(0x24),
    d.Field(d.NullableRef(d.RemoteAddr(d.Array(d.Int32, 0x40000))), "versions"),  -- (version << 18) of each object
    d.Field(d.NullableRef(d.RemoteAddr(d.Array(d.RawAddr(), 0x40000))), "entities"),  -- entities themselves
})

M.PropertyType = d.Struct("PropertyType", {
    d.Field(d.UInt32, "fourcc"),  -- space padded fourcc code (always 4 chars, reverse order)
    d.Field(d.Int32, "fourcc_length"),  -- length of code without padding
    d.Field(d.Int32, "index"),  -- some index into the property manager system (unique for each type)
    d.Field(d.Int32, "size"),  -- size of the data in bytes
    d.Field(d.Int32, "type_flags"),  -- element type bit flags: 1 = bytes, 2 = characters, 4 = ints, 8 = floats, ...
})

--- map PropertyType.fourcc to struct
-- There are more fourcc's registered in the type system but these appear to be the only ones used.
M.property_struct = {
    -- "r": entity manager handle (version << 18) | (index & 0x3FFFF)
    [0x72202020] = d.UInt32,
    -- "gref": global reference handle
    -- a gref is an offset relative to gref_manager.pool.base
    -- the pool itself is relocatable so this gives a stable reference inside the pool
    -- tagged with 0x40000000 to mark as gref (as opposed to regular pointer)
    -- gref = 0x40000000 | (addr - gref_manager.pool.base)
    -- addr = gref_manager.pool.base + (gref & 0x3FFFFFFF)
    [0x67726566] = d.UInt32,
    -- "l": integer
    [0x6c202020] = d.Int32,
    -- "f": float
    [0x66202020] = d.Float,
    -- "b": bool
    [0x62202020] = d.Int32,
    -- "str ": string
    [0x73747220] = d.ZString(0x100),
}

M.Property = d.Struct("Property", {
    d.Field(d.Int32, "key_length"),  -- including terminating null
    d.Field(d.Ref(M.PropertyType), "type"),  -- type of data
    d.Field(d.Int32, "size"),   -- size of the data in bytes
    d.Field(d.ZString(0x40), "key"),  -- the key string
    -- data follows immediately after the key string
    -- offset is not static, need to calculate at runtime: 0x0C + key_length
    -- number of elements is size / type.size
})

local PlayerData = d.Struct("PlayerData", {
    --[[
    d.Seek(0xB61),
    d.Field(d.Int32, "unk_b61"),
    d.Field(d.Int32, "unk_b65"),
    d.Field(d.Int32, "unk_b69"),
    ]]
    d.Seek(0xB6D),
    d.Field(d.Int32, "player_gref"),  -- gref to player that owns this data
    --[[
    d.Field(d.Int32, "unk_gref_b71"),
    d.Field(d.Int32, "unk_b75"),
    d.Field(d.Int32, "unk_b79"),
    d.Skip(0x8),
    d.Field(d.Int32, "unk_b85"),
    d.Field(d.Int32, "unk_b89"),
    d.Field(M.SmallString, "interaction"),  -- text from top left menu
    d.Field(M.SmallString, "unk_c0d"),
    d.Field(d.Int32, "unk_c8d"),
    d.Field(d.Int32, "unk_gref_c91"),
    d.Field(d.Int8, "unk_c95"),
    d.Field(d.Int8, "unk_c96"),
    d.Field(d.Int32, "unk_c97"),
    d.Seek(0xD34),
    d.Field(d.Int32, "unk_d34"),
    d.Field(d.Int8, "unk_d38"),
    d.Field(Vector3, "unk_d39"),
    d.Field(d.Int8, "unk_d45"),
    d.Field(d.Int32, "unk_d46"),
    d.Skip(0x4),
    d.Field(Vector3, "unk_d4e"),
    d.Field(d.Int32, "unk_d5a"),
    d.Field(d.Int32, "unk_d5e"),
    d.Field(d.Int32, "unk_d62"),
    d.Field(d.Float, "unk_d66"),
    d.Field(d.Int32, "unk_d6a"),
    d.Field(d.Int32, "unk_d6e"),
    d.Field(d.Int32, "unk_d72"),
    d.Field(d.Int32, "unk_d76"),
    d.Field(d.Int32, "unk_d7a"),
    d.Field(d.Int32, "unk_d7e"),
    d.Seek(0x108C),
    d.Field(d.RawAddr(), "unk_vtable"),
    d.Seek(0x11C3),
    d.Field(d.UInt32, "unk_11c3"),  -- could also be a gref
    ]]
    d.Seek(0x11C7),
    d.Field(d.Int32, "shots_fired"),
    d.Seek(0x1263),  -- confirmed size
})

M.Player = d.Struct("Player", {
    d.Field(Matrix33, "unk_matrix_00"),
    d.Seek(0x3C),
    d.Field(d.Int32, "unk_flag_3c"), -- bit 0x08000000 means "don't free this object"?
    d.Seek(0x54),
    d.Field(d.Ref(PlayerData), "data"),
    d.Field(d.Int8, "unk_flag_58"),
    d.Seek(0x6C),
    d.Field(d.Int32, "unk_6c"),
    -- was last field, size is likely 0x70
})

M.PlayerEntity = d.Struct("PlayerEntity", {
    d.Seek(0x48),
    -- gref to Player
    d.Field(d.UInt32, "gref"),
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
    d.Field(d.RawAddr(), "vtable"),
    d.Seek(0x14),
    d.Field(d.Ref(GRefManagerPool), "pool"),  -- relocateble objects referenced by gref
    d.Seek(0x24),
    d.Field(d.Int32, "is_allocated"),
    d.Field(d.Int32, "pool_size"),
    d.Seek(0x5D),
    d.Field(d.RawAddr(), "slots"),  -- seems to keep track where things have been allocated, exact structure unknown
})

local SceneEntities = d.Struct("SceneEntities", {
    d.Skip(0x4),
    -- handles must be resolved with the entity_manager
    -- not sure about capacity, 1000 is wild guess
    d.Field(d.Ref(d.RemoteAddr(d.Array(d.Int32, 1000))), "handles"),
    d.Field(d.RawAddr(), "unk_08"),
    d.Field(d.Int32, "unk_0c"),
    d.Field(d.Int32, "num_handles"),
})

local SceneManager = d.Struct("SceneManager", {
    d.Skip(0x4),
    d.Field(d.NullableRef(GRefManager), "gref_manager"),  -- holds relocatable scene memory pool
    d.Seek(0xC4),
    d.Field(d.NullableRef(SceneEntities), "entities"),  -- can be null during mission reload
    d.Seek(0xBB7),
    d.Field(M.SmallString, "scene_name"),
    d.Seek(0x1C4B),
    d.Field(SharedCom, "shared_com"),  -- holds scene properties
})

local Engine = d.Struct("Engine", {
    d.Seek(0x98),
    d.Field(d.Ref(SceneManager), "scene_manager"),
})

-- the property manager data is these items packed together
M.PropertyManagerRecord = d.Struct("PropertyManagerRecord", {
    d.Field(d.Int32, "record_size"),
    d.Field(d.Int8, "is_active"),
    d.Field(d.RemoteAddr(M.Property), "property"),
    -- property has same structure as SharedCom property:
    -- data follows immediately after the key string
    -- offset is not static, need to calculate at runtime
})

M.PropertyManager = d.Struct("PropertyManager", {
    d.Field(d.RawAddr(), "vtable"),
    d.Field(d.Int32, "data_capacity"),  -- total size
    d.Field(d.RawAddr(), "data"),  -- actual data
    d.Field(d.Int32, "data_used"),   -- how much is actually used
})

M.Game = d.Struct("Game", {
    d.Seek(0x2625D4),
    d.Field(d.RawAddr(), "engine_ptr"),  -- always points to engine field
    d.Seek(0x2625DC),
    d.Field(d.RawAddr(), "unk_2a6c54_ptr"),  -- always points to unk_2a6c54 field
    d.Seek(0x297840),
    d.Field(d.Array(M.PropertyType, 18), "property_types"),
    d.Seek(0x2A6C50),
    d.Field(d.Ref(M.EntityManager), "entity_manager"),
    d.Field(d.RawAddr(), "unk_2a6c54"),
    d.Seek(0x2A6C5C),
    d.Field(d.Ref(Engine), "engine"),
    d.Seek(0x2A6C7C),
    d.Field(d.Ref(M.PropertyManager), "property_manager"),
    d.Seek(0x28AA18),
    d.Field(d.ZString(0x40), "lethed"),  -- literal string constant
    d.Seek(0x2B3418),
    d.Field(d.ZString(0x08), "current_level_name"),  -- only during stats screen
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
    d.Field(d.Int32, "time"),  -- only during stats screen
    d.Field(d.Int32, "saves_used"),  -- only during stats screen
})

--- Get all valid property addresses from the SceneManager shared_com container.
M.get_scene_manager_property_addrs = function(scene_manager)
    local addrs = {}
    for _, block in ipairs(scene_manager.shared_com.container.blocks) do
        for i = 1,block.num_properties,1 do
            addrs[#addrs + 1] = block.properties[i]
        end
    end
    return addrs
end

--- Read all PropertyManager records to get their addresses.
M.read_property_manager_property_addrs = function(property_manager, reader, tracer)
    local ok = true
    local addrs = {}
    local offset = 0
    while offset < property_manager.data_used do
        local remote_record = d.remote_value(M.PropertyManagerRecord, property_manager.data + offset)
        local record, ok = read.read(remote_record, reader, tracer)
        if not ok then break end
        addrs[#addrs + 1] = record.property.address
        offset = offset + record.record_size
    end
    return addrs, ok
end

--- Read single SharedCom property and its data at the given address.
M.read_shared_com_property = function(addr, reader, tracer)
    local remote_property = d.remote_value(M.Property, addr)
    local property, ok = read.read(remote_property, reader, tracer)
    if not ok then return nil end
    local struc = M.property_struct[property.type.fourcc]
    if not struc then
        error(string.format("unknown type fourcc: 0x%x", property.type.fourcc))
    end
    local data = {}
    local total_size = property.type.size
    if total_size == 0 then
        -- happens for "str " property
        total_size = property.size
    end
    for offset = 0,property.size-1,total_size do
        local remote_data = d.remote_value(struc, addr + 0x0C + property.key_length + offset)
        data[#data+1], ok = read.read(remote_data, reader, tracer)
        if not ok then return nil end
    end
    return property, data
end

--- Read all SharedCom properties and their data at the given addresses.
M.read_shared_com_properties = function(addrs, reader, tracer)
    local ok = true
    local properties = {}
    for _, addr in ipairs(addrs) do
        local property, data = M.read_shared_com_property(addr, reader, tracer)
        if not property then
            ok = false
            break
        end
        properties[property.key] = {
            type_fourcc = property.type.fourcc,
            data = data,
        }
    end
    return properties, ok
end

-- The level control code and player code are the indices of the entity.
-- The player gref is the offset relative to the gref base.
-- Looks fragile, but level control code and player_gref seem deterministic.
-- The gref is stored without 0x40000000 tag so can be used directly as an offset.
M.level_infos = {
    -- sanctuary
    ["SCENES\\C0-1\\C0-1__MAIN.gms"] = {
        level_control_code = 0x205,
        player_gref = 0x38EE0,
    },
    -- anathema
    ["SCENES\\C1-1\\C1-1__MAIN.gms"] = {
        level_control_code = 0x20E,
        player_gref = 0x68F20,
    },
    -- stakeout
    ["SCENES\\C2-1\\C2-1__MAIN.gms"] = {
        level_control_code = 0x2C9,
        player_gref = 0x9FE10,
    },
    -- kirov  
    ["SCENES\\C2-2\\C2-2__MAIN.gms"] = {
        level_control_code = 0x228,
        player_gref = 0x54A10,
    },
    -- tubeway
    ["SCENES\\C2-3\\C2-3__MAIN.gms"] = {
        level_control_code = 0x4E,
        player_gref = 0x1E610,
    },
    -- invitation  
    ["SCENES\\C2-4\\C2-4__MAIN.gms"] = {
        level_control_code = 0x2E2,
        player_gref = 0x108CC0,
    },
    -- tracking
    ["SCENES\\C3-1\\C3-1__MAIN.gms"] = {
        level_control_code = 0x2EE,
        player_gref = 0x55650,
    },
    -- hidden valley
    ["SCENES\\C3-2a\\C3-2a__MAIN.gms"] = {
        level_control_code = 0x2D2,
        player_gref = 0x5F670,
    },
    -- gates
    ["SCENES\\C3-2b\\C3-2b__MAIN.gms"] = {
        level_control_code = 0x33A,
        player_gref = 0x4EDC0,
    },
    -- showdown  
    ["SCENES\\C3-3\\C3-3__MAIN.gms"] = {
        level_control_code = 0x4DB,
        player_gref = 0x62A10,
    },
    -- basement  
    ["SCENES\\C4-1\\C4-1__MAIN.gms"] = {
        level_control_code = 0x2B4,
        player_gref = 0x77620,
    },
    -- graveyard
    ["SCENES\\C4-2\\C4-2__MAIN.gms"] = {
        level_control_code = 0x3D4,
        player_gref = 0x811E0,
    },
    -- jacuzzi
    ["SCENES\\C4-3\\C4-3__MAIN.gms"] = {
        level_control_code = 0x235,
        player_gref = 0x44630,
    },
    -- bazaar
    ["SCENES\\C5-1\\C5-1__MAIN.gms"] = {
        level_control_code = 0x27B,
        player_gref = 0x3CFA0,
    },
    -- motorcade
    ["SCENES\\C5-2\\C5-2__MAIN.gms"] = {
        level_control_code = 0x100,
        player_gref = 0x35590,
    },
    -- tunnel rat
    ["SCENES\\C5-3\\C5-3__MAIN.gms"] = {
        level_control_code = 0x27B,
        player_gref = 0x4D310,
    },
    -- temple city
    ["SCENES\\C6-1\\C6-1__MAIN.gms"] = {
        level_control_code = 0x191,
        player_gref = 0x6F820,
    },
    -- hannelore
    ["SCENES\\C6-2\\C6-2__MAIN.gms"] = {
        level_control_code = 0x2C2,
        player_gref = 0x60860,
    },
    -- hospitality
    ["SCENES\\C6-3\\C6-3__MAIN.gms"] = {
        level_control_code = 0x25B,
        player_gref = 0xC5110,
    },
    -- revisited
    ["SCENES\\C7-1\\C7-1__MAIN.gms"] = {
        level_control_code = 0x2C0,
        player_gref = 0xA0270,
    },
    -- finale
    ["SCENES\\C8-1\\C8-1__MAIN.gms"] = {
        level_control_code = 0x2,
        player_gref = 0x15B60,
    },
}

M.resolve_gref = function(base, handle)
    if (handle & 0x40000000) == 0 then
        if handle == 0 then
            return 0
        else
            return nil
        end
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

--- Get the level control address from scene properties (similar to how the game does it).
M.get_level_control_addr_1 = function(versions, entities, scene_properties)
    return M.get_property_entity(versions, entities, scene_properties, "LevelControlCode")
end

--- Get the level control address directly from a given level control code (without version marker).
M.get_level_control_addr_2 = function(versions, entities, level_control_code)
    -- level_control_code stored without marker so no need to resolve, can use as index directly
    assert(versions[level_control_code + 1] == 0x40000)
    return entities[level_control_code + 1]
end

--- Read player via a level's stable player_gref (without marker).
M.read_player = function(gref_pool_base, player_gref, reader, tracer)
    -- gref not resolved as passed without marker
    local remote_player = d.remote_value(M.Player, gref_pool_base + player_gref)
    return read.read(remote_player, reader, tracer)
end

--- Calculate aggression from player statistics.
-- Must be strictly less than 3 for silent assassin rating.
-- This means value <= 6.
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
    return 100 * math.tanh(0.005 * value)
end

--- Calculate stealth from player statistics.
-- Must be 85 or more for silent assassin rating.
-- This means value <= 1.
M.measure_stealth = function(level_control)
    -- raw measure of stealth (non-negative)
    local value = level_control.alerts + level_control.close_encounters
    -- convert to [0,100] scale using power law
    return 100 * (0.9 ^ value)
end

return M
