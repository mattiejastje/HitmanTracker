local M = {}

local d = require("mempeep.descriptors")

local layout = {
  steam = {
    name = "Steam",
    offset = {
      global_data = 0xD58C60,
      stats_manager = 0xD61710,
      challenge_manager = 0xD617C0,
      event_manager = 0xE20E40,
      game_data = 0xE212E0,
      level_manager = 0xE21310,
      level = 0xE21394,
      checkpoints_manager = 0xE21580,
      time_manager = 0xE24730,
      movie_manager = 0xE31B80,
      movie_slots = 0xE37E90,
    },
  },
  gog = {
    name = "GOG",
    offset = {
      global_data = 0xCA0840,
      stats_manager = 0xCA92D0,
      challenge_manager = 0xCA9380,
      event_manager = 0xD68A00,
      game_data = 0xD68EA0,
      level_manager = 0xD68ED0,
      level = 0xD68F54,
      checkpoints_manager = 0xD69140,
      time_manager = 0xC88580,
      movie_manager = 0xC87C00,
      movie_slots = 0xC8F774,
    }
  },
}

local NUM_DIFFICULTIES = 5
-- note: not all levels are used
local NUM_LEVELS = 26
-- levels have fewer than 13 checkpoints
-- the 13 is an upper bound from the engine, used in the array of stats values
local NUM_CHECKPOINTS_PER_LEVEL = 13
local MAX_CHALLENGES = 300  -- never seen more than 279
local NUM_PLAYSTYLES = 26
local MAX_PLAYSTYLE_CONDITIONS = 10
local NUM_STATS_VALUES = 100

local String = d.Struct("String", {
  d.Field(d.Int32, "length"),
  d.Field(d.Ref(d.ZString(0x40)), "text"),
})

local LevelData = d.Struct("LevelData", {
  d.Skip(0x08),
  d.Field(d.Int32, "level"),
})

local CheckpointData = d.Struct("CheckpointData", {
  d.Skip(0x38),
  -- score required for shadow rating
  d.Field(d.Int32, "shadow_raw_score_threshold"),
})

local CheckpointNode = d.Struct("CheckpointNode", {
  d.Skip(0x04),
  d.Field(d.NullableRef(CheckpointData), "data"),
})

local CheckpointInfo = d.Struct("CheckpointInfo", {
  d.Skip(0x10),
  d.Field(d.Vector(CheckpointNode, NUM_CHECKPOINTS_PER_LEVEL), "nodes"),
})

local GameDataLevelInfo = d.Struct("GameDataLevelInfo", {
  d.Skip(0x04),
  d.Field(d.Ref(LevelData), "level_data"),
  d.Skip(0x04),
  d.Field(d.NullableRef(CheckpointInfo), "checkpoint_info"), -- null when level is not loaded
  d.Skip(0xA0), -- size 0xB0
})

local GameData = d.Struct("GameData", {
  d.Skip(0x20),
  d.Field(d.Vector(GameDataLevelInfo, NUM_LEVELS), "level_infos"),
})

local LevelManager = d.Struct("LevelManager", {
  -- 0xE21310
  d.Skip(0x04),
  -- 0xE21314
  d.Field(String, "scene"),  -- "assembly:/Scenes/.../*.entity"
  -- 0xE2131C
  d.Field(d.Int32, "game_mode"),
  d.Skip(0x10),
  -- 0xE21330
  d.Field(d.Int32, "checkpoint_index"), -- cached, not always up to date
})

local Checkpoint = d.Struct("Checkpoint", {
  d.Field(d.Int32, "key"), -- could be pointer
  d.Field(d.Int32, "_unknown"), -- could be pointer
})

local Checkpoints = d.Struct("Checkpoints", {
  d.Skip(0x0C),
  d.Field(d.Vector(Checkpoint, NUM_CHECKPOINTS_PER_LEVEL), "checkpoint"),
  d.Skip(0x3C),
  d.Field(d.Int32, "current_key"),
})

local CheckpointsManager = d.Struct("CheckpointsManager", {
  d.Skip(0x28),
  d.Field(d.NullableRef(Checkpoints), "checkpoints"),
})

local TimeManager = d.Struct("TimeManager", {
  d.Skip(0x08),
  -- 0xE24738
  d.Field(d.Int64, "ticks_per_second"),
  -- 0xE24740
  d.Field(d.Int64, "last_time_ticks"),
  -- 0xE24748
  d.Field(d.Int64, "game_time"), -- divide by (1024 * 1024) to get seconds
  -- 0xE24750
  d.Field(d.Int64, "game_time_previous"),
  d.Field(d.Int64, "game_time_delta"),
  d.Field(d.Int64, "real_time"),
  d.Field(d.Int64, "real_time_previous"),
  d.Field(d.Int64, "real_time_delta"),
  d.Field(d.Float, "game_time_multiplier"),
  d.Field(d.Float, "debug_time_multiplier"),
  d.Field(d.Int64, "frame_wait"),
  d.Field(d.Int64, "frame_step"),
  d.Field(d.Int64, "frame_remain"),
  d.Field(d.Int32, "paused"),
  d.Field(d.Int32, "frame_count"),
})

local StatsScoringData = d.Struct("StatsScoringData", {
  d.Skip(0x08),
  d.Field(String, "title"),
  d.Skip(0x18),
  d.Field(d.Float, "_unused"), -- always zero
  d.Skip(0x08),
  d.Field(d.Int32, "index"), -- last index into StatsManager.values
  d.Skip(0x04),
  d.Field(d.Int32, "multiplier"), -- see SCORE_MULTIPLIER below
})

local StatsScoring = d.Struct("StatsScoring", {
  d.Skip(0x04),
  d.Field(d.Ref(StatsScoringData), "data"),
})

-- same layout as StatsScoringData?
local PlaystyleConditionData = d.Struct("PlaystyleConditionData", {
  d.Skip(0x08),
  d.Field(String, "title"),
  d.Skip(0x24),
  d.Field(d.Int32, "index"),
  d.Field(d.Int32, "threshold"),
})

-- same layout as StatsScoring?
local PlaystyleCondition = d.Struct("PlaystyleCondition", {
  d.Skip(0x04),
  d.Field(d.Ref(PlaystyleConditionData), "data"),
})

local StatsPlaystyleData = d.Struct("StatsPlaystyleData", {
  d.Skip(0x08),
  d.Field(d.Vector(PlaystyleCondition, MAX_PLAYSTYLE_CONDITIONS), "condition_min"),
  d.Skip(0x04),
  d.Field(d.Vector(PlaystyleCondition, MAX_PLAYSTYLE_CONDITIONS), "condition_max"),
  d.Skip(0x04),
  d.Field(String, "title"),
  d.Skip(0x0C),
  d.Field(d.Int8, "is_unlockable"), -- if listed under "unlocks"
  d.Skip(0x03),
  d.Field(d.Int32, "priority"), -- higher values = higher ranking
  d.Field(d.Int32, "percentage_min"),
  d.Field(d.Int32, "percentage_max"),
  d.Skip(0x0C),
  d.Field(d.Int8, "is_achieved"), -- for current mission
  d.Skip(0x03),
})

local StatsPlaystyle = d.Struct("StatsPlaystyle", {
  d.Skip(0x04),
  d.Field(d.Ref(StatsPlaystyleData), "data"),
})

local StatsDifficulties = d.Struct("StatsDifficulties", {
  d.Skip(0x08),
  d.Field(d.Array(d.Float, NUM_DIFFICULTIES), "scales"),
})

local StatsManager = d.Struct("StatsManager", {
  d.Skip(0x04),
  d.Field(d.Vector(StatsScoring, NUM_STATS_VALUES), "scorings"),
  d.Skip(0x04),
  d.Field(d.Vector(StatsPlaystyle, NUM_PLAYSTYLES), "playstyles"),
  d.Skip(0x10),
  d.Field(d.Ref(d.RemoteAddr(d.Array(d.Array(d.Array(d.Int16, NUM_STATS_VALUES), NUM_CHECKPOINTS_PER_LEVEL), NUM_LEVELS))), "values"),
  d.Skip(0x08),
  d.Field(d.Ref(d.Array(d.Int8, 100)), "achieved_playstyles"), -- across all gaming sessions
  d.Skip(0x2C),
  d.Field(d.Int32, "last_achieved_playstyle"), -- across all gaming sessions
  d.Skip(0x18),
  -- scaled_score = raw_score * scale from difficulty/challenges
  d.Field(d.Int32, "scaled_score"),
  d.Skip(0x18),
  d.Field(d.NullableRef(StatsDifficulties), "difficulties"), -- 1.0, 1.25, 1.5, 2.0, 2.5; see DIFFICULTY_SCALE below
})

local EventManager = d.Struct("EventManager", {
    d.Seek(0x14),
    -- index 0 = regular kill
    -- index 1 = headshot kill (includes thrown weapon at head)
    d.Field(d.Vector(d.Int32, 5), "events_per_kill_type_class"),
    d.Seek(0x20),
    -- event count per event type; examples include
    -- index 20 = axe kill (thrown or melee)
    -- index 270 = pistol kill
    -- index 580 = fiber wire kill
    -- index 660 = pistol elimination / neck snap kill / poison kill ...
    d.Field(d.Vector(d.Int32, 1337), "events_per_event_type"),
    d.Seek(0x2C),
    -- unknown event counter
    d.Field(d.Vector(d.Int32, 11), "events_per_unknown"),
    d.Seek(0x38),
    -- kill count per npc type
    -- index 0 = unused?
    -- index 1 = civilian
    -- index 2 = guard
    -- index 3 = target
    d.Field(d.Vector(d.Int32, 4), "kills_per_npc_type"),
    d.Seek(0x5C),
    -- index 0x23 = spotted
    d.Field(d.Vector(d.Int32, 51), "events_per_event_type_2"),
    d.Seek(0xFC),
    d.Field(d.Int32, "npcs_killed"),  -- includes targets too
    d.Seek(0x148),
    -- trespass = when exclamation mark shows
    -- happens also when holding a gun as civilian, etc.
    d.Field(d.Int8, "trespass"),
    d.Skip(0x7),
    -- time trespass state activated
    d.Field(d.Int64, "trespass_time"),
    d.Field(d.Vector(d.Int32, 0x1000), "listeners"),
    d.Seek(0x364),
    d.Field(d.Int8, "is_total_count_enabled"),
    d.Skip(0x3),
    d.Field(d.Int32, "total_count"),
})

local ChallengeData = d.Struct("ChallengeData", {
  d.Seek(0x98),
  d.Field(d.Int8, "completed"),
})

local ChallengeNode = d.Struct("ChallengeNode", {
  d.Field(d.RawAddr(), "next_node"),
  d.Skip(0x08),
  d.Field(d.NullableRef(ChallengeData), "data"),
})

local ChallengeManager = d.Struct("ChallengeManager", {
  d.Skip(0x08),
  d.Field(d.List(ChallengeNode, "next_node", d.list_kind.CIRCULAR, MAX_CHALLENGES), "challenges"),
})

local GlobalData = d.Struct("GlobalData", {
  d.Skip(0x94),
  d.Field(d.Bounded(d.Int32, 0, NUM_DIFFICULTIES - 1), "difficulty"),
  d.Skip(0xA4),
  -- unlocks -> statistics -> stealth
  -- 0x13C
  d.Field(d.Int32, "bodies_hidden"),
  -- 0x140
  d.Field(d.Int32, "disguises_picked_up"),
  d.Field(d.Int32, "containers_entered"),
  d.Field(d.Int32, "fuse_boxes_disabled"),
  d.Field(d.Int32, "successful_blend_ins"),
  -- 0x150
  d.Field(d.Int32, "caught_dragging_body"),
  -- caught_trespassing is bugged, counts trespassing even if not caught!
  d.Field(d.Int32, "caught_trespassing"),
  d.Field(d.Int32, "disguises_blown"),
  d.Field(d.Int32, "knockouts_by_hand"),
  -- 0x160
  d.Field(d.Int32, "knockouts_by_improvised_weapons"),
  -- unlocks -> statistics -> kills
  -- 0x164
  d.Field(d.Int32, "npcs_killed"),
  d.Field(d.Int32, "headshots"),
  d.Field(d.Int32, "fiber_wires"),
  -- 0x170
  d.Field(d.Int32, "deadly_throws"),
  d.Field(d.Int32, "kills_with_improvised_weapons"),
  d.Field(d.Int32, "pull_off_ledges"),
  d.Field(d.Int32, "push_over_railings"),
  -- 0x180
  d.Field(d.Int32, "point_shooting_kills"),
  d.Field(d.Int32, "kill_shots_from_cover"),
  d.Field(d.Int32, "action_kills_triggered"),
  d.Field(d.Int32, "kill_shots_pistol"),
  -- 0x190
  d.Field(d.Int32, "kill_shots_sniper"),
  d.Field(d.Int32, "kill_shots_smg"),
  d.Field(d.Int32, "kill_shots_shotgun"),
  d.Field(d.Int32, "kill_shots_assault_rifle"),
  -- 0x1A0
  d.Field(d.Int32, "kill_shots_revolver"),
  d.Field(d.Int32, "civilians_killed"),
  d.Field(d.Int32, "cops_killed"),  -- cop = any guard
  d.Field(d.Int32, "pigs_blown_up"),
  -- 0x1B0
  d.Field(d.Int32, "pigeons_killed"),
})

local MovieInfoBuffer = d.Struct("MovieInfoBuffer", {
  d.Field(d.Int32, "is_allocated"),
  d.Field(d.RawAddr(), "buffer"),
  d.Field(d.Int32, "unk_maybe_width"),
})

local MovieInfo = d.Struct("MovieInfo", {
  d.Field(d.Bounded(d.Int32, 0, 3), "num_allocated_planes"),
  d.Field(d.Int32, "width"),
  d.Field(d.Int32, "height"),
  d.Field(d.Int32, "chroma_width"),
  d.Field(d.Int32, "chroma_height"),
  d.Field(d.Int32, "unk_14"),
  -- each plane consists of 4 buffers
  -- number of planes actually in use is num_allocated_planes
  -- data may be invalid for unused entries
  d.Field(d.Array(d.Array(MovieInfoBuffer, 0x4), 0x3), "planes"),
  d.Field(d.Int32, "unk_a8"),
})

local MovieManagerData = d.Struct("MovieManagerData", {
  d.Skip(0x8),
  d.Field(MovieInfo, "info"),
  d.Seek(0xD8),
  d.Field(d.RawAddr(), "unk_bink_handle"),
  d.Seek(0x11C),
  d.Field(d.Int32, "unk_11c"),
  d.Field(d.Int32, "unk_120"),
  d.Seek(0x12C),
  -- bits 0x1, 0x4, and 0x8 confirmed
  -- bit 0x4 is set when movie is playing
  d.Field(d.Int8, "state_flags"),
  d.Field(d.Int8, "unk_flags_12d"),
})

local MovieManager = d.Struct("MovieManager", {
  d.Seek(0x84),
  d.Field(d.Ref(MovieManagerData), "data"),
})

local game = function(layout)
  return d.Struct(
    "Game" .. layout.name, {
      d.Seek(layout.offset.global_data + 0x10),
      d.Field(GlobalData, "global_data"),
      d.Seek(layout.offset.stats_manager),
      d.Field(StatsManager, "stats_manager"),
      d.Seek(layout.offset.challenge_manager),
      d.Field(ChallengeManager, "challenge_manager"),
      d.Seek(layout.offset.event_manager),
      d.Field(EventManager, "event_manager"),
      d.Seek(layout.offset.game_data),
      d.Field(GameData, "game_data"),
      d.Seek(layout.offset.level_manager),
      d.Field(LevelManager, "level_manager"),
      d.Seek(layout.offset.level),
      -- level == -1 used by game when no level selected
      d.Field(d.Bounded(d.Int32, -1, NUM_LEVELS - 1), "level"), -- part of level manager?
      d.Seek(layout.offset.checkpoints_manager),
      d.Field(CheckpointsManager, "checkpoints_manager"),
      d.Seek(layout.offset.time_manager),
      d.Field(TimeManager, "time_manager"),
      d.Seek(layout.offset.movie_manager),
      d.Field(MovieManager, "movie_manager"),
      d.Seek(layout.offset.movie_slots),
      d.Field(d.Array(d.Int8, 8), "movie_slots"),
    },
    { native_name = "Game" }
  )
end

M.GameSteam = game(layout.steam)
M.GameGOG = game(layout.gog)

M.get_current_checkpoint_index = function(checkpoints)
  if checkpoints.current_key == 0 then
    return -1 -- no checkpoint loaded
  end
  for i, checkpoint in ipairs(checkpoints.checkpoint) do
    if checkpoint.key == checkpoints.current_key then
      return i - 1 -- lua index started at 1
    end
  end
  error("Unable to find checkpoint key")
end

M.get_num_challenges_completed = function(challenges)
  local count = 0
  for _, node in ipairs(challenges) do
    local data = node.data -- occasionally can be nil
    if data and data.completed ~= 0 then
      count = count + 1
    end
  end
  return count
end

-- score multipliers by final index into StatsManager.values
-- note lua index starts at 1
local MULTIPLIERS = {
  0, -- (unknown category)
  5000, -- objective complete
  10000, -- target kill
  -1000, -- spotted
  1000, -- evidence removed
  47000, -- silent assassin bonus
  5000, -- signature kill
  150, -- silent kill
  150, -- headshot
  100, -- body hidden
  -2500, -- civilian casualty
  -250, -- non-target casualty
  -100, -- pacification
}

-- result uses lua indexing
M.get_multipliers = function(scorings)
  local multipliers = {}
  for _, scoring in ipairs(scorings) do
    local data = scoring.data
    if data.multiplier ~= 0 then
      multipliers[data.index + 1] = data.multiplier
    end
  end
  return multipliers
end

-- here values is stats_manager.values[level+1][checkpoint+1]
-- should be array containing 100 entries
M.get_raw_score = function(values, multipliers)
  local sum = 0
  for i, multiplier in pairs(multipliers) do
    sum = sum + values[i] * multiplier
  end
  return sum
end

--- Calculate raw score (before scaling by difficulty/challenges).
-- Uses hardcoded multipliers.
M.get_raw_score_2 = function(values)
  return M.get_raw_score(values, MULTIPLIERS)
end

--- Calculate final score.
-- Uses floating point arithmetic.
-- difficulty_scale is stats_manager.difficulties.scales[difficulty+1]
M.get_scaled_score = function(raw_score, difficulty_scale, num_challenges_completed)
  return math.floor(raw_score * (difficulty_scale + 0.05 * num_challenges_completed) + 0.5)
end

local DIFFICULTY_SCALE = { 100, 125, 150, 200, 250 }

--- Calculate final score.
-- This implementation uses hardcoded scales and exact integer arithmetic.
M.get_scaled_score_2 = function(raw_score, difficulty, num_challenges_completed)
  return (raw_score * (DIFFICULTY_SCALE[difficulty + 1] + 5 * num_challenges_completed) + 50) // 100
end

--- Check if conditions are achieved according to playstyle.
M.is_playstyle_condition_achieved = function(values, playstyle_data)
  if next(playstyle_data.condition_min) == nil and next(playstyle_data.condition_max) == nil then
    return false -- not a condition-based playstyle
  end
  for _, condition in ipairs(playstyle_data.condition_min) do
    local data = condition.data
    if values[data.index] < data.threshold then
      return false
    end
  end
  for _, condition in ipairs(playstyle_data.condition_max) do
    local data = condition.data
    if values[data.index] >= data.threshold then
      return false
    end
  end
  return true
end

--- Return currently achieved condition based playstyle.
-- Condition based playstyles are numbered 5 to 25. Silent assassin is 24.
-- @return The index of the playstyle, or nil if no playstyle is achieved yet.
M.get_playstyle_index_by_condition = function(values, playstyles)
  -- match highest priority playstyles first
  table.sort(playstyles, function(a, b)
    return a.data.priority > b.data.priority
  end)
  for i, playstyle in pairs(playstyles) do
    local data = playstyle.data
    if M.is_playstyle_condition_achieved(values, data) then
      return i - 1
    end
  end
  return nil
end

--- Find level info for given level.
M.get_level_info = function(level_infos, level)
  for _, level_info in ipairs(level_infos) do
    if level_info.level_data.level == level then
      return level_info
    end
  end
  return nil -- not found
end

--- Raw score threshold for "Shadow" rating.
M.get_shadow_raw_score_threshold = function(level_infos, level, checkpoint_index)
  local level_info = M.get_level_info(level_infos, level)
  if level_info ~= nil then
    return level_info.checkpoint_info.nodes[checkpoint_index + 1].data.shadow_raw_score_threshold
  else
    return nil
  end
end

--- Check if percentage is achieved according to playstyle.
M.is_playstyle_percentage_achieved = function(percentage, playstyle_data)
  return (playstyle_data.percentage_min <= percentage and percentage <= playstyle_data.percentage_max)
end

--- Return currently achieved score based playstyle.
-- Score based playstyles are agent (0), veteran (1), specialist (2), professional (3), and shadow (4).
-- @return The index of the playstyle, or nil if no playstyle is achieved yet.
M.get_playstyle_index_by_score = function(raw_score, shadow_raw_score_threshold, playstyles)
  local percentage = math.max(0, math.min(100, (100 * raw_score) // shadow_raw_score_threshold))
  for i, playstyle in ipairs(playstyles) do
    if M.is_playstyle_percentage_achieved(percentage, playstyle.data) then
      return i - 1 -- lua index starts at 1
    end
  end
  return nil -- no match
end

-- use hardcoded bounds
M.get_playstyle_index_by_score_2 = function(raw_score, shadow_raw_score_threshold)
  local percentage = (100 * raw_score) // shadow_raw_score_threshold
  local bounds = { 49, 79, 89, 99 }
  for i, bound in ipairs(bounds) do
    if percentage <= bound then
      return i - 1 -- lua index starts at 1
    end
  end
  return 4
end

return M
