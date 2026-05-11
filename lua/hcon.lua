M = {}

local d = require("mempeep.descriptors")

local SmallString = d.Struct("SmallString", {
    d.Field(d.ZString(0x100), "text"),
    d.Skip(0x7C),  -- inline buffer for strings <= 0x7C chars
})

local EntityManager = d.Struct("EntityManager", {
    -- entity handle = (version << 18) | (index & 0x3FFFF)
    d.Seek(0x24),
    d.Field(d.Ref(d.Array(d.Int32, 0x40000)), "versions"),  -- (version << 18) of each object
    d.Field(d.Ref(d.Array(d.RawAddr(), 0x40000)), "entities"),  -- entities themselves
})

local SceneManager = d.Struct("SceneManager", {
    d.Seek(0xBB0),
    d.Field(d.Int8, "pause_flag_1"),
    d.Field(d.Int8, "pause_flag_2"),
    d.Seek(0xBCD),
    d.Field(SmallString, "scene_name"),
    d.Seek(0x6D39),
    d.Field(d.Int32, "unk_6d39"),
})

local Engine = d.Struct("Engine", {
    d.Seek(0x24),
    d.Field(d.Float, "game_time"),  -- ticks / 1024, only updated 5 times per second
    d.Seek(0x30),
    d.Field(d.Int32, "game_ticks_copy"),
    d.Skip(0x4),
    d.Field(d.Int32, "game_ticks"),  -- in ticks, most accurate, root source of all game timings
    d.Field(d.Int32, "game_ticks_previous"),
    d.Field(d.Float, "frame_time"),  -- smoothed time spent on each frame (seconds)
    d.Field(d.Int32, "pause_ticks_offset"),  -- total time game was paused, in negative ticks
    d.Seek(0xA5),
    d.Field(d.Ref(SceneManager), "scene_manager"),
    d.Seek(0x80D),
    d.Field(d.Float, "game_time_update_interval"),  -- 0.2, interval to update game_time_seconds
})

local PlayerData = d.Struct("PlayerData", {
    d.Seek(0x13DB),
    d.Field(d.Int32, "shots_fired"),
})

local Player = d.Struct("Player", {
    d.Skip(0x8),
    d.Field(d.NullableRef(PlayerData), "data"),  -- null when in menu etc.
})

local StatsManager = d.Struct("StatsManager", {
    d.Seek(0xB17),
    d.Field(d.Int32, "headshots"),
    d.Field(d.Int32, "enemies_wounded"),
    d.Field(d.Int32, "enemies_killed"),
    d.Field(d.Int32, "innocents_wounded"),
    d.Field(d.Int32, "innocents_killed"),
    d.Field(d.Int32, "alerts"),
    d.Field(d.Int32, "close_encounters"),
})

M.HitmanContracts = d.Struct("HitmanContracts", {
    d.Seek(0x30E484),
    d.Field(d.Float, "seconds_per_tick"),  -- 1/1024
    d.Seek(0x30E808),
    d.Field(d.Float, "ticks_per_second"),  -- 1024
    d.Seek(0x394570),
    d.Field(d.Ref(EntityManager), "entity_manager"),
    d.Seek(0x39457C),
    d.Field(d.Ref(Engine), "engine"),
    d.Seek(0x3945A4),
    d.Field(d.Ref(Player), "player"),  -- used as root source for player data by game
    d.Seek(0x3947B0),
    d.Field(d.RawAddr(), "player_data_copy_1"),  -- equal to the player.data pointer
    d.Seek(0x3947C0),
    d.Field(d.NullableRef(StatsManager), "stats_manager"),
    d.Seek(0x395718),
    d.Field(d.RawAddr(), "player_data_copy_2"),  -- equal to the player.data pointer but sometimes stale e.g. when in menu after mission
})

return M
