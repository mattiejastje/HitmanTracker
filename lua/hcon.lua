M = {}

local d = require("mempeep.descriptors")

local SmallString = d.Struct("SmallString", {
    d.Field(d.ZString(0x100), "text"),
    d.Skip(0x7C),  -- inline buffer for strings <= 0x7C chars
})

local SceneManager = d.Struct("SceneManager", {
    d.Seek(0xBCD),
    d.Field(SmallString, "scene_name"),
})

local Engine = d.Struct("Engine", {
    d.Seek(0x24),
    d.Field(d.Float, "game_time"),  -- ticks / 1024, only updated 5 times per second
    d.Seek(0x38),
    d.Field(d.Int32, "game_ticks"),  -- in ticks, most accurate, root source of all game timings
    d.Field(d.Int32, "game_ticks_previous"),
    d.Field(d.Float, "frame_time"),  -- smoothed time spent on each frame (seconds)
    d.Field(d.Int32, "pause_ticks_offset"),  -- total time game was paused, in negative ticks
    d.Seek(0xA5),
    d.Field(d.Ref(SceneManager), "scene_manager"),
    d.Seek(0x80D),
    d.Field(d.Float, "game_time_update_interval"),  -- 0.2, interval to update game_time_seconds
})

M.HitmanContracts = d.Struct("HitmanContracts", {
    d.Seek(0x30E484),
    d.Field(d.Float, "seconds_per_tick"),  -- 1/1024
    d.Seek(0x30E808),
    d.Field(d.Float, "ticks_per_second"),  -- 1024
    d.Seek(0x39457C),
    d.Field(d.Ref(Engine), "engine"),
})

return M
