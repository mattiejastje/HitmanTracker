local M = {}

local d = require("mempeep.descriptors")

local Node = d.Struct("Node", {
    d.Field(d.RawAddr(), "node_data"),
    d.Field(d.Int32, "unk2"),
    d.Field(d.RawAddr(), "next_node"),
})

local EngineData = d.Struct("EngineData", {
    d.Seek(0x59),
    d.Field(d.RawAddr(), "unk_59"),  -- event manager?
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
    d.Seek(0x37E5),
    d.Field(d.List(Node, "next_node", d.list_kind.NULL_TERMINATED, 0x1000), "nodes"),
    d.Seek(0x38F0),
    d.Field(d.Int8, "unk_flag_38f0"),
    d.Field(d.Int8, "timestep_mode"),  -- 1 fixed time (each frame 0.03 seconds), 0 real time
    d.Seek(0x398E),
    d.Field(d.Float, "game_speed_scale"),  -- time scale factor (normally 1.0)
    d.Seek(0x3BCA),
    d.Field(d.Int8, "unk_flag_3bca"),
    d.Field(d.Int64, "cpu_cycles"),
    d.Field(d.Double, "frame_time_smoothed"),  -- smoothed time spent on each frame
    d.Seek(0x3BDB),
    d.Field(d.RawAddr(), "unk_3bdb"),  -- virtual function
})

local Engine = d.Struct("Engine", {
    d.Field(d.Ref(EngineData), "EngineData"),
})

M.HitmanDlc = d.Struct("HitmanDlc", {
    d.Seek(0x1F0008),
    d.Field(d.RawAddr(), "unk_1f0008"),  -- memory manager?
    d.Field(d.Ref(Engine), "engine"),
    d.Seek(0x1F03C4),
    d.Field(d.Float, "unk_zero_1f03c4"),  -- seems always zero
})

return M
