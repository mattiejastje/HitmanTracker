-- lua script for documenting and testing pointers

local NUM_DIFFICULTIES          = 5
local NUM_LEVELS                = 26
local NUM_CHECKPOINTS_PER_LEVEL = 13
local STATS_COUNT               = 100
local MAX_CHALLENGES            = 278  -- actual total number

function safe_func(fn, ...)
    local val = fn(...)
    if val == nil then error("Failed", 2) end
    return val
end

function get_difficulty(base)
    local difficulty = safe_func(readInteger, base + 0xD58D04, true)
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
    }
end

function get_stats_values(stats_manager, level, checkpoint_index)
    local block_index = (level * NUM_CHECKPOINTS_PER_LEVEL) + checkpoint_index
    local block_ptr = stats_manager.values_ptr + (block_index * STATS_COUNT * 2)
    local stats_values = {}
    for i = 1, STATS_COUNT do
        local value = safe_func(readShortInteger, block_ptr, true)
        stats_values[i] = value
        block_ptr = block_ptr + 2  -- each value is 2 bytes
    end
    return stats_values
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
        node_ptr = node.next_node_ptr
        if count > MAX_CHALLENGES then error("Too many challenges") end
    end
    return count
end

function get_raw_score(stats_manager, stats_values)
    local entry_ptr = stats_manager.entry_begin_ptr
    local sum = 0
    local num_entries = 0
    while entry_ptr ~= stats_manager.entry_end_ptr do
        local descriptor_ptr = safe_func(readPointer, entry_ptr + 0x04)
        if descriptor_ptr ~= 0 then
            local index = safe_func(readInteger, descriptor_ptr + 0x34, true)
            if index < 0 or index >= STATS_COUNT then error("Index out of range: " .. index) end
            local multiplier = safe_func(readInteger, descriptor_ptr + 0x3C, true)
            local value = stats_values[index + 1]
            sum = sum + value * multiplier
        end
        num_entries = num_entries + 1
        entry_ptr = entry_ptr + 0x08
        if num_entries > STATS_COUNT then error("Too many entries") end
    end
    return sum
end

local DIFFICULTY_SCALE = {100, 125, 150, 200, 250}

function get_score(raw_score, difficulty, num_challenges_completed)
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
    local num_challenges_completed = get_num_challenges_completed(challenge_manager)
    print("Num challenges completed: " .. num_challenges_completed)
    local checkpoint_manager = get_checkpoint_manager(base)
    if checkpoint_manager.checkpoints_ptr == 0 then return end  -- no level loaded
    local checkpoints = get_checkpoints(checkpoint_manager.checkpoints_ptr)
    local checkpoint_index = get_current_checkpoint_index(checkpoints)
    print("Checkpoint index: " .. checkpoint_index)
    if checkpoint_index < 0 then return end  -- no checkpoint loaded
    local stats_manager = get_stats_manager(base)
    local stats_values = get_stats_values(stats_manager, level, checkpoint_index)
    print("Stats values: " .. table.concat(stats_values, ", "))
    local raw_score = get_raw_score(stats_manager, stats_values)
    print("Raw score: " .. raw_score)
    local score = get_score(raw_score, difficulty, num_challenges_completed)
    print("Score: " .. score)
end

main()
