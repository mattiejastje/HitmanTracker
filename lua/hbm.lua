local M = {}

local d = require("mempeep.descriptors")
local common = require("common")

local layouts = {
  steam = {
    name = "Steam",
    offset = {
      sys_interface = 0x41F820,
      settings = 0x41F83C,
      stats = 0x5B2538,
    },
  },
  gog = {
    name = "GOG",
    offset = {
      sys_interface = 0x420820,
      settings = 0x42083C,
      stats = 0x5B3B38,
    },
  },
}

local SceneInfo = d.Struct("SceneInfo", {
  d.Field(d.Int8, "unk_flag_00"),
  d.Skip(0x1),
  d.Field(d.ZString(260), "scene_name"),
})

local SceneTask = d.Struct("SceneTask", {
  d.Field(d.RawAddr(), "prev_task"),
  d.Field(d.RawAddr(), "next_task"),
  d.Seek(0x0C),
  d.Field(d.Float, "unk_tick_interval"), -- not sure
  d.Field(d.Int32, "unk_last_tick"), -- not sure
  d.Skip(0x4),
  d.Field(d.Int32, "unk_flags_1"), -- not sure
  d.Field(d.Int32, "unk_flags_2"), -- not sure
  d.Field(d.RawAddr(), "unk_ptr_24"),
})

local SceneTaskScheduler = d.Struct("SceneTaskScheduler", {
  d.Skip(0x4), -- vtable?
  d.Field(d.RemoteAddr(d.Array(d.List(SceneTask, "next_task", d.list_kind.CIRCULAR, 0x100), 9)), "tasks"),
  d.Seek(0x54),
  d.Field(d.RawAddr(), "unk_current_54"),
  d.Field(d.RawAddr(), "unk_active_58"),
})

-- view is "" or "Stats"
local make_scene_manager = function(view)
  local is_full = view == ""
  local fields = {}
  common.add_fields(fields, {
    d.Seek(0x24),
    d.Field(d.Ref(SceneInfo), "info"),
  })
  common.add_fields_if(is_full, fields, {
    d.Seek(0x30),
    d.Field(SceneTaskScheduler, "task_scheduler"),
    d.Seek(0xBC),
    d.Field(d.Int8, "is_paused"),
    d.Seek(0x52D0),
    d.Field(d.RawAddr(), "unk_52d0"), -- related to task_scheduler->unk_active_58
  })
  return d.Struct("SceneManager" .. view, fields)
end

-- view is "" or "Stats" or "Timer"
local make_sys_interface = function(view)
  local is_full = view == ""
  local is_stats = view == "Stats"
  local is_timer = view == "Timer"
  local fields = {}
  common.add_fields_if(is_full, fields, {
    d.Field(d.RawAddr(), "vtable"),
    d.Skip(0x0C),
    d.Field(d.Double, "clock_elapsed"), -- elapsed time in current mission (based on clock)
    d.Field(d.Double, "clock_current"), -- "wall clock" time
    d.Field(d.Double, "qpc_elapsed"), -- elapsed time in current mission (based on qpc)
    d.Field(d.Int32, "clock_ticks"), -- clock_elapsed * 1024
    d.Field(d.Float, "clock_delta"), -- scaled frame delta
    d.Field(d.Int32, "clock_ticks_previous"), -- previous clock_ticks
    d.Skip(0x4),
    d.Field(d.Int32, "qpc_ticks"), -- qpc_elapsed * 1024
    d.Field(d.Float, "qpc_delta"), -- scaled frame delta
    d.Field(d.Int32, "game_ticks_copy"), -- unsure, copy of game_ticks
    d.Skip(0x4),
    d.Field(d.Int32, "game_ticks"), -- in ticks, based on qpc_ticks but accounts for pause
    d.Field(d.Int32, "game_ticks_previous"),
    d.Field(d.Float, "game_frame_time"), -- (game_ticks - game_ticks_previous) / 1024.0
    d.Field(d.Int32, "pause_ticks_offset"), -- total time game was paused, in negative ticks
    d.Skip(0x8),
    d.Field(d.Float, "qpc_frequency"), -- 1000000 (number of qpc's per second)
  })
  common.add_fields_if(is_timer, fields, {
    d.Seek(0x48),
    d.Field(d.Int32, "game_ticks"),
  })
  common.add_fields_if(is_full or is_stats, fields, {
    d.Seek(0xB8),
    d.Field(d.NullableRef(make_scene_manager(view)), "scene_manager"),
  })
  common.add_fields_if(is_full, fields, {
    d.Seek(0xB24),
    d.Field(d.Float, "requested_timescale"), -- -1.0 if default
    d.Field(d.Float, "timescale"), -- 1.0
    d.Field(d.Int8, "is_timescale_locked"),
    d.Seek(0xDE1),
    d.Field(d.Int8, "use_qpc"), -- whether to use QueryPerformanceCounter
    d.Seek(0x11F8),
    d.Field(d.Int64, "qpc_time_offset"),
    d.Seek(0x1438),
    d.Field(d.Int64, "qpc_last_sample"), -- result of QueryPerformanceCounter
    d.Field(d.Double, "qpc_frame_time"), -- smoothed time spent on each frame (seconds)
    d.Seek(0x1660),
    d.Field(d.Int8, "qpc_force_tick"),
  })
  return d.Struct("SysInterface" .. view, fields)
end

local Suits = d.Struct("Suits", {
  d.Field(d.Int32, "current_suit"), -- probably a pointer
  d.Field(d.Int32, "starting_suit"), -- probably a pointer
})

local SuitContainer = d.Struct("SuitContainer", {
  d.Seek(0x0FD0),
  d.Field(Suits, "suits"),
})

local Settings = d.Struct("Settings", {
  d.Seek(0x0A40),
  d.Field(d.NullableRef(SuitContainer), "suit_container"), -- non-null during mission
  d.Seek(0x6664),
  d.Field(d.Bounded(d.Int32, 0, 3), "difficulty"),
})

-- view is "" or "Stats" or "Timer"
local make_game = function(layout, view)
  local is_full = view == ""
  local is_stats = view == "Stats"
  local is_timer = view == "Timer"
  local fields = {}
  common.add_fields(fields, {
    d.Seek(layout.offset.sys_interface),
    d.Field(d.NullableRef(make_sys_interface(view)), "sys_interface"),
  })
  common.add_fields_if(is_full or is_stats, fields, {
    d.Seek(layout.offset.settings),
    d.Field(d.NullableRef(Settings), "settings"), -- null when game starts
    d.Seek(layout.offset.stats),
    d.Field(d.Array(d.Int32, 66), "stats"),
  })
  common.add_fields_if(is_timer, fields, {
    d.Seek(layout.offset.stats + 39 * 4),
    d.Field(d.Int32, "stats_time"),
  })
  common.add_fields_if(is_full, fields, {
    d.Seek(0x356108),
    d.Field(d.Float, "seconds_per_tick"), -- 1/1024
    d.Seek(0x35ECD0),
    d.Field(d.Double, "seconds_per_millisecond"), -- 1/1000
  })
  return d.Struct("Game" .. layout.name .. view, fields, { native_name = "Game" .. view })
end

M.GameSteam = make_game(layouts.steam, "")
M.GameGOG = make_game(layouts.gog, "")
M.GameSteamStats = make_game(layouts.steam, "Stats")
M.GameGOGStats = make_game(layouts.gog, "Stats")
M.GameSteamTimer = make_game(layouts.steam, "Timer")
M.GameGOGTimer = make_game(layouts.gog, "Timer")

return M
