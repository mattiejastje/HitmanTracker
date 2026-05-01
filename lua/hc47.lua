local M = {}

local d = require("mempeep.descriptors")

local Node = d.Struct("Node", {
    d.Field(d.RawAddr(), "node_data"),
    d.Field(d.Int32, "unk2"),
    d.Field(d.RawAddr(), "next_node"),
})

local EngineData = d.Struct("EngineData", {
    d.Seek(0x59),
    d.Field(d.RawAddr(), "unknown"),  -- event manager?
    d.Seek(0x37E5),
    d.Field(d.List(Node, "next_node", d.list_kind.NULL_TERMINATED, 0x1000), "nodes"),
})

local Engine = d.Struct("Engine", {
    d.Field(d.Ref(EngineData), "EngineData"),
})

M.HitmanDlc = d.Struct("HitmanDlc", {
    d.Seek(0x1F0008),
    d.Field(d.RawAddr(), "unknown"),  -- memory manager?
    d.Field(d.Ref(Engine), "engine"),
})

return M
