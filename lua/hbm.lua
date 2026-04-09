local M = {}

local d = require("mempeep.descriptors")

local TimeManager = d.Struct("TimeManager", {
  d.Seek(0x48),
  d.Field(d.Int32, "time"),
})

local Suits = d.Struct("Suits", {
  d.Field(d.Int32, "current_suit"),  -- probably a pointer
  d.Field(d.Int32, "starting_suit"),  -- probably a pointer
});

local SuitContainer = d.Struct("SuitContainer", {
  d.Seek(0x0FD0),
  d.Field(d.Ref(Suits), "suits")
})

local Settings = d.Struct("Settings", {
  d.Seek(0x0A40),
  d.Field(d.Ref(SuitContainer), "suit_container"),
  d.Seek(0x6664),
  d.Field(d.Int32, "difficulty")
})

M.Game = d.Struct("Game", {
  d.Seek(0x41F820),
  d.Field(d.Ref(TimeManager), "time_manager"),
  d.Seek(0x41F83C),
  d.Field(d.Ref(Settings), "settings"),
  d.Seek(0x5B2538),
  d.Field(d.PrimitiveArray("i4", { 66 }), "stats"),
})

return M
