local M = {}

local d = require("mempeep.descriptors")

local SmallString = d.Struct("SmallString", {
    d.Field(d.ZString(0x100), "text"),
    d.Skip(0x7C),  -- inline buffer for strings <= 0x7C chars
})

local SceneNode = d.Struct("SceneNode", {
    d.Seek(-0x108),
    d.Field(d.Int8, "unk_flag_1"),
    d.Field(d.Int8, "unk_flag_2"),
    d.Field(d.Ref(SmallString), "scene_name"),
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
    d.Field(d.RawAddr(), "scenes_tail"),  -- pointer to oldest scene on stack (current mission)
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
    d.Seek(0x398E),
    d.Field(d.Float, "game_speed_scale"),  -- time scale factor (normally 1.0)
    d.Seek(0x3BCA),
    d.Field(d.Int8, "unk_flag_3bca"),
    d.Field(d.Int64, "cpu_cycles"),
    d.Field(d.Double, "frame_time_smoothed"),  -- smoothed time spent on each frame
    d.Field(d.RawAddr(), "unk_3bdb"),  -- virtual function
})

local Engine = d.Struct("Engine", {
    d.Field(d.Ref(EngineData), "EngineData"),
})

M.HitmanDlc = d.Struct("HitmanDlc", {
    d.Seek(0x1F0008),
    d.Field(d.RawAddr(), "unk_1f0008"),  -- memory manager?
    d.Field(d.Ref(Engine), "engine"),
    d.Field(d.RawAddr(), "unk_1f0010"),  -- ?
    d.Seek(0x1F03C4),
    d.Field(d.Float, "unk_zero_1f03c4"),  -- seems always zero
})

return M
