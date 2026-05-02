local M = {}

local d = require("mempeep.descriptors")

local SmallString = d.Struct("SmallString", {
    d.Field(d.ZString(0x100), "text"),
    d.Skip(0x7C),  -- inline buffer for strings <= 0x7C chars
})

local SceneNode = d.Struct("SceneNode", {
    d.Seek(-0x10C),  -- (2 - link_offset) * 4 and link_offset is 0x45
    d.Skip(0x4),
    d.Field(d.Int8, "unk_flag_1"),
    d.Field(d.Int8, "unk_flag_2"),
    d.Field(d.Ref(SmallString), "scene_name"),
    d.Seek(-0x10C + 0xDA),
    d.Field(d.Double, "scene_creation_time"),
    d.Seek(0),
    d.Field(d.RawAddr(), "prev_node"), -- toward newer/head
    d.Field(d.RawAddr(), "next_node"); -- toward older nodes
})

local SceneContainerVTable = d.Struct("SceneContainerVTable", {
    d.Seek(0x3C),
    d.Field(d.RawAddr(), "unk_func_3c"),
    d.Seek(0x48),
    d.Field(d.RawAddr(), "unk_func_48"),
})

local SceneContainer = d.Struct("SceneContainer", {
    d.Field(d.Ref(SceneContainerVTable), "vtable"),
    d.Skip(0x8),
    d.Field(d.UInt32, "flags"),
    d.Skip(0x8),
    d.Field(d.Int32, "link_offset"),  -- always 0x45? actual offset is (2 - link_offset) * 4
    d.Field(d.List(SceneNode, "next_node", d.list_kind.NULL_TERMINATED, 0x100), "scenes"),  -- from newest scene to oldest
    d.Field(d.RawAddr(), "scenes_tail"),  -- pointer root scene on stack (current mission)
})

local SceneManager = d.Struct("SceneManager", {
    d.Seek(0x7D),
    d.Field(d.Int8, "unk_flag_7d"),  -- "dirty" flag? guards if scene needs restoring
    d.Field(d.NullableRef(SceneContainer), "scene_container"),
})

local UnkNodeVTable = d.Struct("UnkNodeVTable", {
    d.Seek(0x98),
    d.Field(d.RawAddr(), "unk_func_98"),
    d.Field(d.RawAddr(), "unk_func_9C"),
})

local UnkNode = d.Struct("UnkNode", {
    d.Field(d.Ref(UnkNodeVTable), "vtable"),
    d.Field(d.Int32, "unk2"),
    d.Field(d.RawAddr(), "next_node"),
})

local PropertyManager = d.Struct("PropertyManager", {
    d.Seek(0x0D),
    d.Field(d.RawAddr(), "unk_data"),
})

local EngineData = d.Struct("EngineData", {
    d.Seek(0x59),
    d.Field(d.Ref(SceneManager), "scene_manager"),  -- scene manager?
    d.Seek(0x2E1),
    d.Field(d.ZString(0x40), "mounted_archive"),  -- always "Intro.zip"
    d.Seek(0xA61),
    d.Field(d.Double, "cpu_cycles_per_second"),
    d.Skip(0x8),
    d.Field(d.Float, "unk_a71"),  -- compared with HitmanDlc+1F03C4 i.e. 0
    d.Field(d.Float, "unk_a75"),  -- compared with HitmanDlc+1F03C4 i.e. 0
    d.Seek(0x37B5),
    d.Field(d.Double, "game_time"),  -- total time with scaling/smoothing
    d.Field(d.Double, "real_time"),  -- total time without scaling/smoothing
    d.Field(d.Double, "game_time_copy"),  -- seems to hold a copy of game_time?
    d.Field(d.Double, "game_time_previous"),  -- game_time from previous frame
    d.Skip(0x10),
    d.Field(d.List(UnkNode, "next_node", d.list_kind.NULL_TERMINATED, 0x1000), "unk_nodes"),
    d.Seek(0x38F0),
    d.Field(d.Int8, "unk_flag_38f0"),
    d.Field(d.Int8, "timestep_mode"),  -- 1 fixed time (each frame 0.03 seconds), 0 real time
    d.Seek(0x3905),
    d.Field(d.Ref(PropertyManager), "property_manager"),
    d.Seek(0x398E),
    d.Field(d.Float, "game_speed_scale"),  -- time scale factor (normally 1.0)
    d.Seek(0x3BCA),
    d.Field(d.Int8, "unk_flag_3bca"),
    d.Field(d.Int64, "cpu_cycles"),
    d.Field(d.Double, "frame_time_smoothed"),  -- smoothed time spent on each frame
    d.Field(d.RawAddr(), "unk_3bdb"),  -- virtual function
})

local Engine = d.Struct("Engine", {
    d.Field(d.Ref(EngineData), "engine_data"),
})

local Unk1f0008Data = d.Struct("Unk1f0008Data", {
    d.Seek(0x0D),
    d.Field(d.Array(d.RawAddr(), 1), "unk_table"),  -- actually much larger, at least 0x1C00
})

local Unk1f0008 = d.Struct("Unk1f0008", {
    d.Field(d.Ref(Unk1f0008Data), "unk_1f0008_data"),
})

M.HitmanDlc = d.Struct("HitmanDlc", {
    d.Seek(0x1F0008),
    d.Field(d.Ref(Unk1f0008), "unk_1f0008"),  -- memory manager?
    d.Field(d.Ref(Engine), "engine"),
    d.Field(d.RawAddr(), "unk_1f0010"),  -- ?
    d.Seek(0x1F03C4),
    d.Field(d.Float, "unk_zero_1f03c4"),  -- seems always zero
    d.Seek(0x245E1C),
    d.Field(d.Float, "unk_float_245e1c"),
    d.Skip(0x8),
    d.Field(d.Int32, "unk_counter_1"),  -- wraps around 8
    d.Skip(0x4),
    d.Field(d.Double, "unk_game_time_1"),  -- sometimes tracks game_time, sometimes not
    d.Field(d.Int32, "unk_counter_2"),  -- wraps around 8
    d.Skip(0x4),
    d.Field(d.Double, "unk_game_time_2"),  -- sometimes tracks game_time, sometimes not
})

return M
