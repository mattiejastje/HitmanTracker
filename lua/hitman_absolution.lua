local typedefs = require("typedefs")
local T = typedefs.T

-- ----------------------------------------------------------------------------
-- Game data structure
-- ----------------------------------------------------------------------------

struct_defs = {
    Game = {
        size = 0x7FFFFFFF,
        fields = {
            {name = "difficulty", offset = 0xD58C60 + 0x10 + 0x94, type_ref = T.i32},
            {name = "game_data", offset = 0xE212E0, type_ref = T.struct("GameData")},
            {name = "level_manager", offset = 0xE21310, type_ref = T.struct("LevelManager")},
            {name = "level", offset = 0xE21394, type_ref = T.i32},  -- part of level manager?
            {name = "checkpoints_manager", offset = 0xE21580, type_ref = T.struct("CheckpointsManager")},
            {name = "time_manager", offset = 0xE24730, type_ref = T.struct("TimeManager")},
            {name = "stats_manager",     offset = 0xD61710, type_ref = T.struct("StatsManager")},
            {name = "challenge_manager", offset = 0xD617C0, type_ref = T.struct("ChallengeManager")},
        }
    },
    String = {
        size = 0x08,
        fields = {
            {name = "length", offset = 0x00, type_ref = T.i32},
            {name = "text",   offset = 0x04, type_ref = T.ptr(T.string(0x40))},
        },
    },
    GameData = {
        size = 0x28,
        fields = {
            {name = "level_infos", offset = 0x20, type_ref = T.vector(T.struct("GameDataLevelInfo"))},
        }
    },
    GameDataLevelInfo = {
        size = 0xB0,
        fields = {
            {name = "level_data", offset = 0x04, type_ref = T.ptr(T.struct("LevelData"))},
            {name = "checkpoint_info", offset = 0x0C, type_ref = T.optional_ptr(T.struct("CheckpointInfo"))},  -- null during missions
        }
    },
    LevelData = {
        size = 0x0C,
        fields = {
            {name = "level", offset = 0x08, type_ref = T.i32},
        }
    },
    CheckpointInfo = {
        size = 0x18,
        fields = {
            {name = "nodes", offset = 0x10, type_ref = T.vector(T.struct("CheckpointNode"))},
        }
    },
    CheckpointNode = {
        size = 0x08,
        fields = {
            {name = "data", offset = 0x04, type_ref = T.ptr(T.struct("CheckpointData"))},
        }
    },
    CheckpointData = {
        size = 0x40,
        fields = {
            {name = "best_raw_score", offset = 0x38, type_ref = T.i32},
        }
    },
    LevelManager = {
        size = 0x24,
        fields = {
            {name = "scene", offset = 0x04, type_ref = T.struct("String")},
            {name = "game_mode", offset = 0x0C, type_ref = T.i32},
            {name = "checkpoint_index", offset = 0x20, type_ref = T.i32},  -- cached, not always up to date
        },      
    },
    CheckpointsManager = {
        size = 0x2C,
        fields = {
            {name = "checkpoints", offset = 0x28, type_ref = T.optional_ptr(T.struct("Checkpoints"))}
        }
    },
    Checkpoints = {
        size = 0x54,
        fields = {
            {name = "checkpoint", offset = 0x0C, type_ref = T.vector(T.struct("Checkpoint"))},
            {name = "current_key", offset = 0x50, type_ref = T.i32}
        }
    },
    Checkpoint = {
        size = 0x08,
        fields = {
            {name = "key", offset = 0x00, type_ref = T.i32}, -- could be pointer
            {name = "_unknown", offset = 0x04, type_ref = T.i32} -- could be pointer
        }
    },
    TimeManager = {
        size = 0x88,
        fields = {
            {name = "ticks_per_second", offset = 0x08, type_ref = T.i64},
            {name = "last_time_ticks", offset = 0x10, type_ref = T.i64},
            {name = "game_time", offset = 0x18, type_ref = T.i64},  -- divide by (1024 * 1024) to get seconds
            {name = "game_time_previous", offset = 0x20, type_ref = T.i64},
            {name = "game_time_delta", offset = 0x28, type_ref = T.i64},
            {name = "real_time", offset = 0x30, type_ref = T.i64},
            {name = "real_time_previous", offset = 0x38, type_ref = T.i64},
            {name = "real_time_delta", offset = 0x40, type_ref = T.i64},
            {name = "game_time_multiplier", offset = 0x48, type_ref = T.float},
            {name = "debug_time_multiplier", offset = 0x4C, type_ref = T.float},
            {name = "frame_wait", offset = 0x50, type_ref = T.i64},
            {name = "frame_step", offset = 0x58, type_ref = T.i64},
            {name = "frame_remain", offset = 0x60, type_ref = T.i64},
            {name = "paused", offset = 0x68, type_ref = T.i32},
            {name = "frame_count", offset = 0x6C, type_ref = T.i32},
        }
    },
    StatsManager = {
        size = 0xA0,
        fields = {
            {name = "scorings", offset = 0x04, type_ref = T.vector(T.struct("StatsScoring"))},
            {name = "playstyles", offset = 0x10, type_ref = T.vector(T.struct("StatsPlaystyle"))},
            {name = "values", offset = 0x28, type_ref = T.ptr(T.array(T.array(T.array(T.i16, 100), 13), 26)), print = false},
            {name = "achieved_playstyles", offset = 0x34, type_ref = T.ptr(T.array(T.i8, 100))},  -- across all gaming sessions
            {name = "last_achieved_playstyle", offset = 0x64, type_ref = T.i32},  -- across all gaming sessions
            {name = "score",  offset = 0x80, type_ref = T.i32},
            {name = "difficulties", offset = 0x9C, type_ref = T.ptr(T.struct("StatsDifficulties"))},
        }
    },
    StatsScoring = {
        size = 0x08,
        fields = {
            {name = "data", offset = 0x04, type_ref = T.ptr(T.struct("StatsScoringData"))},
        }
    },
    StatsScoringData = {
        size = 0x40,
        fields = {
            {name = "title",      offset = 0x08, type_ref = T.struct("String")},
            {name = "_unused",    offset = 0x28, type_ref = T.float},  -- always zero
            {name = "index",      offset = 0x34, type_ref = T.i32},    -- final index into StatsManager.values
            {name = "multiplier", offset = 0x3C, type_ref = T.i32},    -- see SCORE_MULTIPLIER below
        }
    },
    StatsDifficulties = {
        size = 0x1C,
        fields = {
            {name = "scales", offset = 0x08, type_ref = T.array(T.float, 5)},  -- 1.0, 1.25, 1.5, 2.0, 2.5; see DIFFICULTY_SCALE below
        }
    },
    StatsPlaystyle = {
        size = 0x08,
        fields = {
            {name = "data", offset = 0x04, type_ref = T.ptr(T.struct("StatsPlaystyleData"))},
        }
    },
    StatsPlaystyleData = {
        size = 0x54,
        fields = {
            {name = "condition_min",  offset = 0x08, type_ref = T.vector(T.struct("PlaystyleCondition"))},
            {name = "condition_max",  offset = 0x14, type_ref = T.vector(T.struct("PlaystyleCondition"))},
            {name = "title",          offset = 0x20, type_ref = T.struct("String")},
            {name = "is_unlockable",  offset = 0x34, type_ref = T.i8},  -- if listed under "unlocks"
            {name = "priority",       offset = 0x38, type_ref = T.i32}, -- higher values = higher ranking
            {name = "percentage_min", offset = 0x3C, type_ref = T.i32},
            {name = "percentage_max", offset = 0x40, type_ref = T.i32},
            {name = "is_achieved",    offset = 0x50, type_ref = T.i8},  -- for current mission
        }
    },
    PlaystyleCondition = {
        size = 0x08,
        fields = {
            {name = "data", offset = 0x04, type_ref = T.ptr(T.struct("PlaystyleConditionData"))},
        }
    },
    -- same layout as StatsScoringData?
    PlaystyleConditionData = {
        size = 0x3C,
        fields = {
            {name = "title",     offset = 0x08, type_ref = T.struct("String")},
            {name = "index",     offset = 0x34, type_ref = T.i32},
            {name = "threshold", offset = 0x38, type_ref = T.i32},
        }
    },
    ChallengeManager = {
        size = 0x10,
        fields = {
            {name = "challenges", offset = 0x08, type_ref = T.circular_list(T.struct("ChallengeNode"), "next_node")},
        }
    },
    ChallengeNode = {
        size = 0x10,
        fields = {
            {name = "next_node", offset = 0x00, type_ref = T.weak_ptr(T.struct("ChallengeNode"))},
            {name = "data",      offset = 0x0C, type_ref = T.optional_ptr(T.struct("ChallengeData"))},
        }
    },
    ChallengeData = {
        size = 0xA0, -- unknown
        fields = {
            {name = "completed", offset = 0x98, type_ref = T.i8},
        }
    },
}

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

hitman_absolution.struct_defs                      = struct_defs
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
