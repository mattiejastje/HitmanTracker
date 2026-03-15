mempeep = require("mempeep")

local T = mempeep.T
local D = mempeep.D

local structs = D.assert_valid({
  D.struct("String", {
    D.field("length", T.i32),
    D.field("text", T.ptr(T.string(0x40))),
  }),
  D.struct("GameData", {
    D.pad(0x20),
    D.field("level_infos", T.vector(T.struct("GameDataLevelInfo"))),
  }),
  D.struct("LevelData", {
    D.pad(0x08),
    D.field("level", T.i32),
  }),
  D.struct("CheckpointData", {
    D.pad(0x38),
    D.field("best_raw_score", T.i32),
  }),
  D.struct("CheckpointNode", {
    D.pad(0x04),
    D.field("data", T.ptr(T.struct("CheckpointData"))),
  }),
  D.struct("CheckpointInfo", {
    D.pad(0x10),
    D.field("nodes", T.vector(T.struct("CheckpointNode"))),
  }),
  D.struct("GameDataLevelInfo", {
    D.pad(0x04),
    D.field("level_data", T.ptr(T.struct("LevelData"))),
    D.pad(0x04),
    D.field("checkpoint_info", T.ptr(T.struct("CheckpointInfo"), {optional = true})),  -- null during missions
    D.pad(0xA0),  -- size 0xB0
  }),
  D.struct("LevelManager", {
    D.pad(0x04),
    D.field("scene", T.struct("String")),
    D.field("game_mode", T.i32),
    D.pad(0x10),
    D.field("checkpoint_index", T.i32),  -- cached, not always up to date
  }),
  D.struct("CheckpointsManager", {
    D.pad(0x28),
    D.field("checkpoints", T.ptr(T.struct("Checkpoints"), {optional = true})),
  }),
  D.struct("Checkpoints", {
    D.pad(0x0C),
    D.field("checkpoint", T.vector(T.struct("Checkpoint"))),
    D.pad(0x3C),
    D.field("current_key", T.i32),
  }),
  D.struct("Checkpoint", {
    D.field("key", T.i32), -- could be pointer
    D.field("_unknown", T.i32), -- could be pointer
  }),
  D.struct("TimeManager", {
    D.pad(0x08),
    D.field("ticks_per_second", T.i64),
    D.field("last_time_ticks", T.i64),
    D.field("game_time", T.i64),  -- divide by (1024 * 1024) to get seconds
    D.field("game_time_previous", T.i64),
    D.field("game_time_delta", T.i64),
    D.field("real_time", T.i64),
    D.field("real_time_previous", T.i64),
    D.field("real_time_delta", T.i64),
    D.field("game_time_multiplier", T.f32),
    D.field("debug_time_multiplier", T.f32),
    D.field("frame_wait", T.i64),
    D.field("frame_step", T.i64),
    D.field("frame_remain", T.i64),
    D.field("paused", T.i32),
    D.field("frame_count", T.i32),
    D.offset(0x88),  -- size
  }),
  D.struct("StatsScoringData", {
    D.pad(0x08),
    D.field("title", T.struct("String")),
    D.pad(0x18),
    D.field("_unused", T.f32),     -- always zero
    D.pad(0x08),
    D.field("index", T.i32),       -- last index into StatsManager.values
    D.pad(0x04),
    D.field("multiplier", T.i32),  -- see SCORE_MULTIPLIER below
  }),
  D.struct("StatsScoring", {
    D.pad(0x04),
    D.field("data", T.ptr(T.struct("StatsScoringData"))),
  }),
  -- same layout as StatsScoringData?
  D.struct("PlaystyleConditionData", {
    D.pad(0x08),
    D.field("title", T.struct("String")),
    D.pad(0x24),
    D.field("index", T.i32),
    D.field("threshold", T.i32),
  }),
  D.struct("PlaystyleCondition", {
    D.pad(0x04),
    D.field("data", T.ptr(T.struct("PlaystyleConditionData"))),
  }),
  D.struct("StatsPlaystyleData", {
    D.pad(0x08),
    D.field("condition_min", T.vector(T.struct("PlaystyleCondition"))),
    D.pad(0x04),
    D.field("condition_max", T.vector(T.struct("PlaystyleCondition"))),
    D.pad(0x04),
    D.field("title", T.struct("String")),
    D.pad(0x0C),
    D.field("is_unlockable", T.i8),  -- if listed under "unlocks"
    D.pad(0x03),
    D.field("priority", T.i32),  -- higher values = higher ranking
    D.field("percentage_min", T.i32),
    D.field("percentage_max", T.i32),
    D.pad(0x0C),
    D.field("is_achieved", T.i8),  -- for current mission
    D.pad(0x03),
  }),
  D.struct("StatsPlaystyle", {
    D.pad(0x04),
    D.field("data", T.ptr(T.struct("StatsPlaystyleData"))),
  }),
  D.struct("StatsDifficulties", {
    D.pad(0x08),
    D.field("scales", T.array(T.f32, 5)),  -- 1.0, 1.25, 1.5, 2.0, 2.5; see DIFFICULTY_SCALE below
  }),
  D.struct("StatsManager", {
    D.pad(0x04),
    D.field("scorings", T.vector(T.struct("StatsScoring"))),
    D.pad(0x04),
    D.field("playstyles", T.vector(T.struct("StatsPlaystyle"))),
    D.pad(0x10),
    D.field("values", T.ptr(T.array(T.array(T.array(T.i16, 100), 13), 26)), { print = false }),
    D.pad(0x08),
    D.field("achieved_playstyles", T.ptr(T.array(T.i8, 100))),  -- across all gaming sessions
    D.pad(0x2C),
    D.field("last_achieved_playstyle", T.i32),  -- across all gaming sessions
    D.pad(0x18),
    D.field("score",  T.i32),
    D.pad(0x18),
    D.field("difficulties", T.ptr(T.struct("StatsDifficulties"), { optional = true })),
  }),
  D.struct("ChallengeData", {
    D.pad(0x98),
    D.field("completed", T.i8),
  }),
  D.struct("ChallengeNode", {
    D.field("next_node", T.ptr(T.struct("ChallengeNode"), { weak = true })),
    D.pad(0x08),
    D.field("data", T.ptr(T.struct("ChallengeData"), { optional = true })),
  }),
  D.struct("ChallengeManager", {
    D.pad(0x08),
    D.field("challenges", T.circular_list(T.struct("ChallengeNode"), "next_node")),
  }),
  D.struct("Game", {
    D.offset(0xD58C60 + 0x10 + 0x94),
    D.field("difficulty", T.i32),
    D.offset(0xD61710),
    D.field("stats_manager", T.struct("StatsManager")),
    D.offset(0xD617C0),
    D.field("challenge_manager", T.struct("ChallengeManager")),
    D.offset(0xE212E0),
    D.field("game_data", T.struct("GameData")),
    D.offset(0xE21310),
    D.field("level_manager", T.struct("LevelManager")),
    D.offset(0xE21394),
    D.field("level", T.i32),  -- part of level manager?
    D.offset(0xE21580),
    D.field("checkpoints_manager", T.struct("CheckpointsManager")),
    D.offset(0xE24730),
    D.field("time_manager", T.struct("TimeManager")),
  }),
})

local schema = mempeep.schema.new(4, structs)


-- ----------------------------------------------------------------------------
-- Helper functions
-- ----------------------------------------------------------------------------

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

-- use hardcoded multipliers
local function get_raw_score_2(values)
    return get_raw_score(values, MULTIPLIERS)
end

-- difficulty_scale is stats_manager.difficulties.scales[difficulty+1]
local function get_score(raw_score, difficulty_scale, num_challenges_completed)
    return math.floor(raw_score * (difficulty_scale + 0.05 * num_challenges_completed) + 0.5)
end

local DIFFICULTY_SCALE = {100, 125, 150, 200, 250}

-- use hardcoded scales and exact integer arithmetic
local function get_score_2(raw_score, difficulty, num_challenges_completed)
    return (raw_score * (DIFFICULTY_SCALE[difficulty + 1] + 5 * num_challenges_completed) + 50) // 100
end

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

local function get_level_info(level_infos, level)
    for _, level_info in ipairs(level_infos) do
        if level_info.level_data.level == level then
            return level_info
        end
    end
    return nil  -- not found
end

local function get_best_raw_score(level_infos, level, checkpoint_index)
    local level_info = get_level_info(level_infos, level)
    return level_info.checkpoint_info.datas[checkpoint_index + 1].sub_data.best_raw_score
end

local function is_playstyle_percentage_achieved(percentage, playstyle_data)
    return (
        playstyle_data.percentage_min <= percentage
        and percentage <= playstyle_data.percentage_max
    )
end

local function get_playstyle_index_by_score(raw_score, best_raw_score, playstyles)
    local percentage = max(0, min(100, raw_score // best_raw_score))
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

-- ----------------------------------------------------------------------------
-- Public API
-- ----------------------------------------------------------------------------

local hitman_absolution = {}

hitman_absolution.schema                           = schema
hitman_absolution.get_current_checkpoint_index     = get_current_checkpoint_index
hitman_absolution.get_num_challenges_completed     = get_num_challenges_completed
hitman_absolution.get_multipliers                  = get_multipliers
hitman_absolution.get_raw_score                    = get_raw_score
hitman_absolution.get_raw_score_2                  = get_raw_score_2
hitman_absolution.get_score                        = get_score
hitman_absolution.get_score_2                      = get_score_2
hitman_absolution.is_playstyle_condition_achieved  = is_playstyle_condition_achieved
hitman_absolution.get_playstyle_index_by_condition = get_playstyle_index_by_condition
hitman_absolution.get_level_info                   = get_level_info
hitman_absolution.get_best_raw_score               = get_best_raw_score
hitman_absolution.is_playstyle_percentage_achieved = is_playstyle_percentage_achieved
hitman_absolution.get_playstyle_index_by_score     = get_playstyle_index_by_score

return hitman_absolution
