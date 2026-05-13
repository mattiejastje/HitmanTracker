M = {}

local d = require("mempeep.descriptors")

local SmallString = d.Struct("SmallString", {
    d.Field(d.Ref(d.ZString(0x100)), "text"),
    d.Skip(0x7C),  -- inline buffer for strings <= 0x7C chars
})

M.PlayerEntity = d.Struct("PlayerEntity", {
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

local SceneManager = d.Struct("SceneManager", {
    d.Seek(0xBB7),
    d.Field(SmallString, "scene_name"),
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
M.mission_player_entity_index = {
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
M.get_player_entity_addr = function(entity_manager, mission_index)
    entity_index = M.mission_player_entity_index[mission_index]
    if entity_index == -1 then return nil end
    -- +1 due to lua indexing
    assert(entity_manager.versions[entity_index + 1] == 0x40000)
    return entity_manager.entities[entity_index + 1]
end

return M
