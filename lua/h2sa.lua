M = {}

local d = require("mempeep.descriptors")

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
    d.Field(d.Ref(d.Array(d.Int32, 0x40000)), "versions"),  -- (version << 18) of each object
    d.Field(d.Ref(d.Array(d.RawAddr(), 0x40000)), "entities"),  -- entities themselves
})

local PropertyType = d.Struct("PropertyType", {
    d.Field(d.UInt32, "fourcc"),  -- space padded fourcc code (always 4 chars, reverse order)
    d.Field(d.Int32, "fourcc_length"),  -- length of code without padding
    d.Field(d.Int32, "unk_id"),  -- unique number, maybe some id or index?
    d.Field(d.Int32, "size"),  -- size of the data in bytes
    d.Field(d.Int32, "unk_elem_type"),  -- maybe element type: 1 = bytes, 2 = characters, 4 = ints, 8 = floats, ...?
})

M.Property = d.Struct("Property", {
    d.Field(d.Int32, "key_length"),  -- including terminating null
    d.Field(d.Ref(PropertyType), "type"),  -- fourcc identifying the type
    d.Field(d.Int32, "size"),   -- size of the data
    d.Field(d.ZString(0x100), "key"),  -- the key string
    -- data follows immediately after the key string
    -- offset is not static, need to calculate at runtime: 0x0C + key_length
})

local PropertyBlock = d.Struct("PropertyBlock", {
    d.Field(d.RawAddr(), "prev_block"),  -- address of previous block (null if first)
    d.Field(d.RawAddr(), "next_block"),  -- address of next block (null if last)
    d.Field(d.Int32, "num_properties"),  -- number of properties in this block
    d.Field(d.Int32, "tombstone_marker"),  -- property pointers beyond num_properties may be equal to this value (though not always)
    d.Field(d.Array(d.RawAddr(), 0x20), "properties")  --- size is max_num_properties_per_block * property_size * 4
})

local SharedComContainer = d.Struct("SharedComContainer", {
    d.Field(d.RawAddr(), "vtable"),
    d.Field(d.List(PropertyBlock, "next_block", d.list_kind.NULL_TERMINATED, 0x1000), "blocks"),
    d.Field(d.RawAddr(PropertyBlock), "last_block"),
    d.Field(d.Int32, "unk_flags"),  -- unknown, always 0x20, 0x80000000 is cleared on access
    d.Field(d.Bounded(d.Int32, 0x20, 0x20), "max_num_properties_per_block"),  -- maximum number of properties per block (always 0x20)
    d.Field(d.Int32, "num_properties_total"),  -- total number of properties across all blocks
    d.Field(d.Bounded(d.Int32, 1, 1), "property_size"), -- always 1
})

local SharedCom = d.Struct("SharedCom", {
    d.Seek(0x4008),
    d.Field(SharedComContainer, "container"),
})

local SceneManager = d.Struct("SceneManager", {
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
})

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

-- player entity index appears to be deterministic for each mission
-- possibly quite fragile, would be nice if we did not need this...
M.level_control_entity_index = {
    -1,  -- unknown / not relevant
    0x20E, 0x2C9, 0x228, 0x4E, 0x2E2, 0x2EE, 0x2D2, 0x33A, 0x4DB, 0x2B4,
    0x3D4, 0x235, 0x27B, 0x100, 0x27B, 0x191, 0x2C2, 0x25B, 0x2C0, 0x2
}

-- return mission as index between 1 and 21 (or nil if not a mission name)
M.get_mission_index = function(scene_name)
    for i, v in ipairs(M.mission_scene_names) do
        if v == scene_name then
            return i
        end
    end
    return nil
end

-- mission_index is number from 1 to 21 (1 for training, 2 for first real mission)
M.get_level_control_entity_addr = function(entity_manager, mission_index)
    entity_index = M.level_control_entity_index[mission_index]
    if entity_index == -1 then return nil end
    -- +1 due to lua indexing
    assert(entity_manager.versions[entity_index + 1] == 0x40000)
    return entity_manager.entities[entity_index + 1]
end

return M
