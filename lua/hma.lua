local M = {}

local d = require("mempeep.descriptors")

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
  d.Field(d.ZString(0x40), "text"),
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
  d.Skip(0x04),
  d.Field(String, "scene"),
  d.Field(d.Int32, "game_mode"),
  d.Skip(0x10),
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
  d.Field(d.Int64, "ticks_per_second"),
  d.Field(d.Int64, "last_time_ticks"),
  d.Field(d.Int64, "game_time"), -- divide by (1024 * 1024) to get seconds
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
  d.Field(d.Ref(d.Array(d.Array(d.Array(d.Int16, NUM_STATS_VALUES), NUM_CHECKPOINTS_PER_LEVEL), NUM_LEVELS)), "values"),
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
  d.Field(d.CircularList(ChallengeNode, "next_node", MAX_CHALLENGES), "challenges"),
})

M.Game = d.Struct("Game", {
  d.Seek(0xD58C60 + 0x10 + 0x94),
  d.Field(d.Bounded(d.Int32, 0, NUM_DIFFICULTIES - 1), "difficulty"),
  d.Seek(0xD61710),
  d.Field(StatsManager, "stats_manager"),
  d.Seek(0xD617C0),
  d.Field(ChallengeManager, "challenge_manager"),
  d.Seek(0xE212E0),
  d.Field(GameData, "game_data"),
  d.Seek(0xE21310),
  d.Field(LevelManager, "level_manager"),
  d.Seek(0xE21394),
  -- level == -1 used by game when no level selected
  d.Field(d.Bounded(d.Int32, -1, NUM_LEVELS - 1), "level"), -- part of level manager?
  d.Seek(0xE21580),
  d.Field(CheckpointsManager, "checkpoints_manager"),
  d.Seek(0xE24730),
  d.Field(TimeManager, "time_manager"),
})

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
