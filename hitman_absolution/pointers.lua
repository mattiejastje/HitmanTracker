-- lua script for documenting and testing pointers

local NUM_DIFFICULTIES          = 5
local NUM_LEVELS                = 26
local NUM_CHECKPOINTS_PER_LEVEL = 13
local STATS_COUNT               = 100
local MAX_CHALLENGES            = 278  -- actual total number

-- multipliers by descriptor index
-- note the game only has non-zero multipliers for indices 1 to 12
-- lua starts indexing at 1
local SCORE_MULTIPLIERS = {
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

function print_table(t)
    for k,v in pairs(t) do
        print("> " .. k .. " = " .. v)
    end
end

function safe_func(fn, ...)
    local val = fn(...)
    if val == nil then error("Failed", 2) end
    return val
end

function get_difficulty(base)
    local difficulty = safe_func(readInteger, base + 0xD58C60 + 0x10 + 0x94, true)
    if difficulty < 0 or difficulty >= NUM_DIFFICULTIES then error("Difficulty " .. difficulty .. " out of bounds") end
    return difficulty
end

function get_level(base)
    local level = safe_func(readInteger, base + 0xE21394, true)
    if level < -1 or level >= NUM_LEVELS then error("Level " .. level .. " out of bounds") end
    return level
end

function get_checkpoint_manager(base)
    local checkpoint_manager_ptr = base + 0xE21580
    return {
        checkpoints_ptr = safe_func(readPointer, checkpoint_manager_ptr + 0x28)
    }
end

function get_checkpoints(checkpoints_ptr)
    return {
        entry_begin_ptr = safe_func(readPointer, checkpoints_ptr + 0x0C),
        entry_end_ptr = safe_func(readPointer, checkpoints_ptr + 0x10),
        current_key = safe_func(readInteger, checkpoints_ptr + 0x50, true),
    }
end

function get_checkpoint_entry(entry_ptr)
    return {
        key = safe_func(readInteger, entry_ptr + 0x00, true),
    }
end

function get_current_checkpoint_index(checkpoints)
    if checkpoints.current_key == 0 then return -1 end  -- no checkpoint loaded
    local index = 0
    local entry_ptr = checkpoints.entry_begin_ptr
    while entry_ptr ~= checkpoints.entry_end_ptr and index < NUM_CHECKPOINTS_PER_LEVEL do
        local entry = get_checkpoint_entry(entry_ptr)
        if entry.key == checkpoints.current_key then return index end
        entry_ptr = entry_ptr + 8  -- each checkpoint entry is 8 bytes
        index = index + 1
    end
    error("Unable to find checkpoint key")
end

function get_stats_manager(base)
    local stats_manager_ptr = base + 0xD61710
    return {
        entry_begin_ptr = safe_func(readPointer, stats_manager_ptr + 0x04),
        entry_end_ptr = safe_func(readPointer, stats_manager_ptr + 0x08),
        values_ptr = safe_func(readPointer, stats_manager_ptr + 0x28),
        score = safe_func(readInteger, stats_manager_ptr + 0x80, true),
        difficulties_ptr = safe_func(readPointer, stats_manager_ptr + 0x9C),
    }
end

function get_stats_values(stats_values_ptr, level, checkpoint_index)
    local block_index = (level * NUM_CHECKPOINTS_PER_LEVEL) + checkpoint_index
    local block_ptr = stats_values_ptr + (block_index * STATS_COUNT * 2)
    local stats_values = {}
    for i = 1, STATS_COUNT do
        local value = safe_func(readShortInteger, block_ptr, true)
        stats_values[i] = value
        block_ptr = block_ptr + 2  -- each value is 2 bytes
    end
    return stats_values
end

function get_stats_difficulties(difficulties_ptr)
    return {
        -- score scale for difficulty 0 (easy) to 4 (purist)
        scale0 = safe_func(readFloat, difficulties_ptr + 0x08),  -- 1.0
        scale1 = safe_func(readFloat, difficulties_ptr + 0x0C),  -- 1.25
        scale2 = safe_func(readFloat, difficulties_ptr + 0x10),  -- 1.5
        scale3 = safe_func(readFloat, difficulties_ptr + 0x14),  -- 2.0
        scale4 = safe_func(readFloat, difficulties_ptr + 0x18),  -- 2.5
    }
end

function get_challenge_manager(base)
    local challenge_manager_ptr = base + 0xD617C0
    return {
        head_node_ptr = challenge_manager_ptr + 0x08
    }
end

function get_challenge_node(node_ptr)
    return {
        next_node_ptr = safe_func(readPointer, node_ptr + 0x00),
        data_ptr = safe_func(readPointer, node_ptr + 0x0C),
    }
end

function get_challenge_data(data_ptr)
    return {
        completed = safe_func(readByte, data_ptr + 0x98, true),
    }
end

function get_num_challenges_completed(challenge_manager)
    local node_ptr = safe_func(readPointer, challenge_manager.head_node_ptr)
    local count = 0
    while node_ptr ~= challenge_manager.head_node_ptr do
        local node = get_challenge_node(node_ptr)
        local data = get_challenge_data(node.data_ptr)
        if data.completed ~= 0 then count = count + 1 end
        if count > MAX_CHALLENGES then error("Too many challenges") end
        node_ptr = node.next_node_ptr
    end
    return count
end

function get_stats_entry(entry_ptr)
    return {
        descriptor_ptr = safe_func(readPointer, entry_ptr + 0x04),
    }
end

function get_stats_descriptor(descriptor_ptr)
    local descriptor = {
        index = safe_func(readInteger, descriptor_ptr + 0x34, true),
        multiplier = safe_func(readInteger, descriptor_ptr + 0x3C, true),
    }
    if descriptor.index < 0 or descriptor.index >= STATS_COUNT then error("Index out of range: " .. descriptor.index) end
    return descriptor
end

-- just for debugging
function print_stats_multipliers(stats_manager)
    local entry_ptr = stats_manager.entry_begin_ptr
    local num_entries = 0
    local multipliers = {}
    while entry_ptr ~= stats_manager.entry_end_ptr do
        local entry = get_stats_entry(entry_ptr)
        if entry.descriptor_ptr ~= 0 then
            local descriptor = get_stats_descriptor(entry.descriptor_ptr)
            if descriptor.multiplier ~= 0 then
                if multipliers[descriptor.index] ~= nil then error("Duplicate descriptor index") end
                multipliers[descriptor.index] = descriptor.multiplier
            end
        end
        num_entries = num_entries + 1
        entry_ptr = entry_ptr + 0x08
        if num_entries > STATS_COUNT then error("Too many entries") end
    end
    print_table(multipliers)
end

function get_raw_score(stats_manager, stats_values)
    local entry_ptr = stats_manager.entry_begin_ptr
    local sum = 0
    local num_entries = 0
    while entry_ptr ~= stats_manager.entry_end_ptr do
        local entry = get_stats_entry(entry_ptr)
        if entry.descriptor_ptr ~= 0 then
            local descriptor = get_stats_descriptor(entry.descriptor_ptr)
            sum = sum + stats_values[descriptor.index + 1] * descriptor.multiplier
        end
        num_entries = num_entries + 1
        entry_ptr = entry_ptr + 0x08
        if num_entries > STATS_COUNT then error("Too many entries") end
    end
    return sum
end

function get_raw_score_fast(stats_values)
    local sum = 0
    for i,v in ipairs(stats_values) do
        local index = i - 1 -- since lua starts counting from 1
        if 1 <= index and index <= 12 then
            sum = sum + v * SCORE_MULTIPLIERS[index]
        end
    end
    return sum
end

local DIFFICULTY_SCALE = {100, 125, 150, 200, 250}

function get_score(raw_score, difficulty, num_challenges_completed)
    -- game uses high precision floats, we use exact integer arithmetic
    -- in practice this should give the same result
    return (raw_score * (DIFFICULTY_SCALE[difficulty + 1] + 5 * num_challenges_completed) + 50) // 100
end

function main()
    print("---")
    local base = getAddressSafe("HMA.exe")
    if base == nil then error("HMA.exe not found") end
    local difficulty = get_difficulty(base)
    print("Difficulty: " .. difficulty)
    local level = get_level(base)
    print("Level: " .. level)
    local challenge_manager = get_challenge_manager(base)
    print("Challenge manager:")
    print_table(challenge_manager)
    local num_challenges_completed = get_num_challenges_completed(challenge_manager)
    print("Num challenges completed: " .. num_challenges_completed)
    local stats_manager = get_stats_manager(base)
    print("Stats manager:")
    print_table(stats_manager)
    local stats_difficulties = get_stats_difficulties(stats_manager.difficulties_ptr)
    print("Stats difficulties:")
    print_table(stats_difficulties)
    print("Stats multipliers:")
    print_stats_multipliers(stats_manager)
    local checkpoint_manager = get_checkpoint_manager(base)
    if checkpoint_manager.checkpoints_ptr == 0 then return end  -- no level loaded
    local checkpoints = get_checkpoints(checkpoint_manager.checkpoints_ptr)
    local checkpoint_index = get_current_checkpoint_index(checkpoints)
    print("Checkpoint index: " .. checkpoint_index)
    if checkpoint_index < 0 then return end  -- no checkpoint loaded
    local stats_values = get_stats_values(stats_manager.values_ptr, level, checkpoint_index)
    print("Stats values: " .. table.concat(stats_values, ", "))
    local raw_score = get_raw_score(stats_manager, stats_values)
    local raw_score_fast = get_raw_score_fast(stats_values)
    if raw_score ~= raw_score_fast then
        error("Raw score mismatch: fast calculation gives " .. raw_score_fast .. " but should be " .. raw_score)
    end
    local score = get_score(raw_score, difficulty, num_challenges_completed)
    if score ~= stats_manager.score then
        error("Calculated score does not match in-game score")
    end
    print("Score: " .. score)
end

main()
