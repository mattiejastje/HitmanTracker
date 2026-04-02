local M = {}

local d = require("mempeep.descriptors")

local MAX_VEC = 0x1000  -- reasonable upper bound for all vectors/lists

local i8   = d.Primitive("i1")
local i16  = d.Primitive("i2")
local i32  = d.Primitive("i4")
local i64  = d.Primitive("i8")
local f32  = d.Primitive("f")

local String = d.Struct("String", {
  d.Field(i32, "length"),
  d.Field(d.Ref(d.Primitive("c64")), "text")
})

local LevelData = d.Struct("LevelData", {
  d.Skip(0x08),
  d.Field(i32, "level")
})

local CheckpointData = d.Struct("CheckpointData", {
  d.Skip(0x38),
  d.Field(i32, "best_raw_score")
})

local CheckpointNode = d.Struct("CheckpointNode", {
  d.Skip(0x04),
  d.Field(d.Ref(CheckpointData), "data")
})

local CheckpointInfo = d.Struct("CheckpointInfo", {
  d.Skip(0x10),
  d.Field(d.Vector(CheckpointNode, MAX_VEC), "nodes")
})

local GameDataLevelInfo = d.Struct("GameDataLevelInfo", {
  d.Skip(0x04),
  d.Field(d.Ref(LevelData), "level_data"),
  d.Skip(0x04),
  d.Field(d.NullableRef(CheckpointInfo), "checkpoint_info"),  -- null when level is not loaded
  d.Skip(0xA0)  -- size 0xB0
})

local GameData = d.Struct("GameData", {
  d.Skip(0x20),
  d.Field(d.Vector(GameDataLevelInfo, MAX_VEC), "level_infos")
})

local LevelManager = d.Struct("LevelManager", {
  d.Skip(0x04),
  d.Field(String, "scene"),
  d.Field(i32, "game_mode"),
  d.Skip(0x10),
  d.Field(i32, "checkpoint_index")  -- cached, not always up to date
})

local Checkpoint = d.Struct("Checkpoint", {
  d.Field(i32, "key"),  -- could be pointer
  d.Field(i32, "_unknown")  -- could be pointer
})

local Checkpoints = d.Struct("Checkpoints", {
  d.Skip(0x0C),
  d.Field(d.Vector(Checkpoint, MAX_VEC), "checkpoint"),
  d.Skip(0x3C),
  d.Field(i32, "current_key")
})

local CheckpointsManager = d.Struct("CheckpointsManager", {
  d.Skip(0x28),
  d.Field(d.NullableRef(Checkpoints), "checkpoints")
})

local TimeManager = d.Struct("TimeManager", {
  d.Skip(0x08),
  d.Field(i64, "ticks_per_second"),
  d.Field(i64, "last_time_ticks"),
  d.Field(i64, "game_time"),  -- divide by (1024 * 1024) to get seconds
  d.Field(i64, "game_time_previous"),
  d.Field(i64, "game_time_delta"),
  d.Field(i64, "real_time"),
  d.Field(i64, "real_time_previous"),
  d.Field(i64, "real_time_delta"),
  d.Field(f32, "game_time_multiplier"),
  d.Field(f32, "debug_time_multiplier"),
  d.Field(i64, "frame_wait"),
  d.Field(i64, "frame_step"),
  d.Field(i64, "frame_remain"),
  d.Field(i32, "paused"),
  d.Field(i32, "frame_count")
})

local StatsScoringData = d.Struct("StatsScoringData", {
  d.Skip(0x08),
  d.Field(String, "title"),
  d.Skip(0x18),
  d.Field(f32, "_unused"),    -- always zero
  d.Skip(0x08),
  d.Field(i32, "index"),      -- last index into StatsManager.values
  d.Skip(0x04),
  d.Field(i32, "multiplier")  -- see SCORE_MULTIPLIER below
})

local StatsScoring = d.Struct("StatsScoring", {
  d.Skip(0x04),
  d.Field(d.Ref(StatsScoringData), "data")
})

-- same layout as StatsScoringData?
local PlaystyleConditionData = d.Struct("PlaystyleConditionData", {
  d.Skip(0x08),
  d.Field(String, "title"),
  d.Skip(0x24),
  d.Field(i32, "index"),
  d.Field(i32, "threshold")
})

-- same layout as StatsScoring?
local PlaystyleCondition = d.Struct("PlaystyleCondition", {
  d.Skip(0x04),
  d.Field(d.Ref(PlaystyleConditionData), "data")
})

local StatsPlaystyleData = d.Struct("StatsPlaystyleData", {
  d.Skip(0x08),
  d.Field(d.Vector(PlaystyleCondition, MAX_VEC), "condition_min"),
  d.Skip(0x04),
  d.Field(d.Vector(PlaystyleCondition, MAX_VEC), "condition_max"),
  d.Skip(0x04),
  d.Field(String, "title"),
  d.Skip(0x0C),
  d.Field(i8,  "is_unlockable"),  -- if listed under "unlocks"
  d.Skip(0x03),
  d.Field(i32, "priority"),  -- higher values = higher ranking
  d.Field(i32, "percentage_min"),
  d.Field(i32, "percentage_max"),
  d.Skip(0x0C),
  d.Field(i8,  "is_achieved"),  -- for current mission
  d.Skip(0x03)
})

local StatsPlaystyle = d.Struct("StatsPlaystyle", {
  d.Skip(0x04),
  d.Field(d.Ref(StatsPlaystyleData), "data")
})

local StatsDifficulties = d.Struct("StatsDifficulties", {
  d.Skip(0x08),
  d.Field(d.Array(f32, 5), "scales")
})

local StatsManager = d.Struct("StatsManager", {
  d.Skip(0x04),
  d.Field(d.Vector(StatsScoring,   MAX_VEC), "scorings"),
  d.Skip(0x04),
  d.Field(d.Vector(StatsPlaystyle, MAX_VEC), "playstyles"),
  d.Skip(0x10),
  d.Field(d.Ref(d.Array(d.Array(d.Array(i16, 100), 13), 26)), "values"),
  d.Skip(0x08),
  d.Field(d.Ref(d.Array(i8, 100)), "achieved_playstyles"),  -- across all gaming sessions
  d.Skip(0x2C),
  d.Field(i32, "last_achieved_playstyle"),  -- across all gaming sessions
  d.Skip(0x18),
  d.Field(i32, "score"),
  d.Skip(0x18),
  d.Field(d.NullableRef(StatsDifficulties), "difficulties")  -- 1.0, 1.25, 1.5, 2.0, 2.5; see DIFFICULTY_SCALE below
})

local ChallengeData = d.Struct("ChallengeData", {
  d.Skip(0x98),
  d.Field(i8, "completed")
})

local ChallengeNode = d.Struct("ChallengeNode", {
  d.Field(d.RawAddr(), "next_node"),
  d.Skip(0x08),
  d.Field(d.NullableRef(ChallengeData), "data")
})

local ChallengeManager = d.Struct("ChallengeManager", {
  d.Skip(0x08),
  d.Field(d.CircularList(ChallengeNode, "next_node", MAX_VEC), "challenges")
})

M.Game = d.Struct("Game", {
  d.Seek(0xD58C60 + 0x10 + 0x94),
  d.Field(i32, "difficulty"),
  d.Seek(0xD61710),
  d.Field(StatsManager, "stats_manager"),
  d.Seek(0xD617C0),
  d.Field(ChallengeManager, "challenge_manager"),
  d.Seek(0xE212E0),
  d.Field(GameData, "game_data"),
  d.Seek(0xE21310),
  d.Field(LevelManager, "level_manager"),
  d.Seek(0xE21394),
  d.Field(i32, "level"),  -- part of level manager?
  d.Seek(0xE21580),
  d.Field(CheckpointsManager, "checkpoints_manager"),
  d.Seek(0xE24730),
  d.Field(TimeManager, "time_manager")
})

local function get_current_checkpoint_index(checkpoints)
    if checkpoints.current_key == 0 then
        return -1  -- no checkpoint loaded
    end
    for i, checkpoint in ipairs(checkpoints.checkpoint) do
        if checkpoint.key == checkpoints.current_key then
            return i - 1  -- lua index started at 1
        end
    end
    error("Unable to find checkpoint key")
end

local function get_num_challenges_completed(challenges)
    local count = 0
    for _, node in ipairs(challenges) do
        local data = node.data  -- occasionally can be nil
        if data and data.completed ~= 0 then count = count + 1 end
    end
    return count
end

-- score multipliers by final index into StatsManager.values
-- note lua index starts at 1
local MULTIPLIERS = {
    0,      -- (unknown category)
    5000,   -- objective complete
    10000,  -- target kill
    -1000,  -- spotted
    1000,   -- evidence removed
    47000,  -- silent assassin bonus
    5000,   -- signature kill
    150,    -- silent kill
    150,    -- headshot
    100,    -- body hidden
    -2500,  -- civilian casualty
    -250,   -- non-target casualty
    -100,   -- pacification
}

-- result uses lua indexing
local function get_multipliers(scorings)
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
local function get_raw_score(values, multipliers)
    local sum = 0
    for i, multiplier in pairs(multipliers) do
        sum = sum + values[i] * multiplier
    end
    return sum
end

--- Calculate raw score (before scaling by difficulty/challenges).
-- Uses hardcoded multipliers.
local function get_raw_score_2(values)
    return get_raw_score(values, MULTIPLIERS)
end

--- Calculate final score.
-- Uses floating point arithmetic.
-- difficulty_scale is stats_manager.difficulties.scales[difficulty+1]
local function get_score(raw_score, difficulty_scale, num_challenges_completed)
    return math.floor(raw_score * (difficulty_scale + 0.05 * num_challenges_completed) + 0.5)
end

local DIFFICULTY_SCALE = {100, 125, 150, 200, 250}

--- Calculate final score.
-- This implementation uses hardcoded scales and exact integer arithmetic.
local function get_score_2(raw_score, difficulty, num_challenges_completed)
    return (raw_score * (DIFFICULTY_SCALE[difficulty + 1] + 5 * num_challenges_completed) + 50) // 100
end

--- Check if conditions are achieved according to playstyle.
local function is_playstyle_condition_achieved(values, playstyle_data)
    if next(playstyle_data.condition_min) == nil and next(playstyle_data.condition_max) == nil then
        return false  -- not a condition-based playstyle
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
local function get_playstyle_index_by_condition(values, playstyles)
    -- match highest priority playstyles first
    table.sort(playstyles, function(a, b) return a.data.priority > b.data.priority end)
    for i, playstyle in pairs(playstyles) do
        local data = playstyle.data
        if is_playstyle_condition_achieved(values, data) then
            return i - 1
        end
    end
    return nil
end

--- Find level info for given level.
local function get_level_info(level_infos, level)
    for _, level_info in ipairs(level_infos) do
        if level_info.level_data.level == level then
            return level_info
        end
    end
    return nil  -- not found
end

--- Raw score needed for best score based playstyle (i.e. "Shadow").
local function get_best_raw_score(level_infos, level, checkpoint_index)
    local level_info = get_level_info(level_infos, level)
    return level_info.checkpoint_info.nodes[checkpoint_index + 1].data.best_raw_score
end

--- Check if percentage is achieved according to playstyle.
local function is_playstyle_percentage_achieved(percentage, playstyle_data)
    return (
        playstyle_data.percentage_min <= percentage
        and percentage <= playstyle_data.percentage_max
    )
end

--- Return currently achieved score based playstyle.
-- Score based playstyles are agent (0), veteran (1), specialist (2), professional (3), and shadow (4).
-- @return The index of the playstyle, or nil if no playstyle is achieved yet.
local function get_playstyle_index_by_score(raw_score, best_raw_score, playstyles)
    local percentage = math.max(0, math.min(100, raw_score // best_raw_score))
    for i, playstyle in ipairs(playstyles) do
        if is_playstyle_percentage_achieved(percentage, playstyle.data) then
            return i - 1  -- lua index starts at 1
        end
    end
    return nil  -- no match
end

-- use hardcoded bounds
local function get_playstyle_index_by_score_2(raw_score, best_raw_score)
    local percentage = raw_score // best_raw_score
    local bounds = {49, 79, 89, 99}
    for i, bound in ipairs(bounds) do
        if percentage <= bound then
            return i - 1  -- lua index starts at 1
        end
    end
    return 4
end

M.get_current_checkpoint_index     = get_current_checkpoint_index
M.get_num_challenges_completed     = get_num_challenges_completed
M.get_multipliers                  = get_multipliers
M.get_raw_score                    = get_raw_score
M.get_raw_score_2                  = get_raw_score_2
M.get_score                        = get_score
M.get_score_2                      = get_score_2
M.is_playstyle_condition_achieved  = is_playstyle_condition_achieved
M.get_playstyle_index_by_condition = get_playstyle_index_by_condition
M.get_level_info                   = get_level_info
M.get_best_raw_score               = get_best_raw_score
M.is_playstyle_percentage_achieved = is_playstyle_percentage_achieved
M.get_playstyle_index_by_score     = get_playstyle_index_by_score
M.get_playstyle_index_by_score_2   = get_playstyle_index_by_score_2

return M
