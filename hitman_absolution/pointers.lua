-- lua script for documenting and testing pointers

local NUM_DIFFICULTIES          = 5
local NUM_LEVELS                = 26
local NUM_CHECKPOINTS_PER_LEVEL = 13
local STATS_COUNT               = 100
local STAT_SIZE_BYTES           = 2

function safeRead(name, fn, addr, ...)
    local val = fn(addr, ...)
    if val == nil then error("Unable to read " .. name, 2) end
    return val
end

function getDifficulty(base)
    local difficulty = safeRead("difficulty", readInteger, base + 0xD58D04, true)
    if difficulty < 0 or difficulty >= NUM_DIFFICULTIES then error("Difficulty " .. difficulty .. " out of bounds") end
    return difficulty
end

function getLevel(base)
    local level = safeRead("level", readInteger, base + 0xE21394, true)
    if level < -1 or level >= NUM_LEVELS then error("Level " .. level .. " out of bounds") end
    return level
end

function getCheckpoint(base)
    local checkpointsPtr = safeRead("checkpoints pointer", readPointer, base + 0xE21580 + 0x28)
    if checkpointsPtr == 0 then return -1 end  -- no level loaded
    local key = safeRead("current checkpoint key", readInteger, checkpointsPtr + 0x50)
    if key == 0 then return -1 end  -- no checkpoint loaded
    local checkpoint = 0
    local entryPtr = safeRead("first entry pointer", readPointer, checkpointsPtr + 0x0C)
    local entryEndPtr = safeRead("end entry pointer", readPointer, checkpointsPtr + 0x10)
    while entryPtr ~= entryEndPtr and checkpoint < NUM_CHECKPOINTS_PER_LEVEL do
        local entryKey = safeRead("checkpoint key", readInteger, entryPtr)
        if entryKey == key then return checkpoint end
        entryPtr = entryPtr + 8  -- each checkpoint entry is 8 bytes
        checkpoint = checkpoint + 1
    end
    error("Unable to find current checkpoint key")
end

function getStatsManager(base)
    local statsManagerPtr = base + 0xD61710
    return {
        entryBeginPtr = safeRead("entry begin pointer", readPointer, statsManagerPtr + 0x04),
        entryEndPtr = safeRead("entry end pointer", readPointer, statsManagerPtr + 0x08),
        valuesPtr = safeRead("stats values pointer", readPointer, statsManagerPtr + 0x28),
    }
end

function getStatsValues(statsManager, level, checkpoint)
    local blockIndex = (level * NUM_CHECKPOINTS_PER_LEVEL) + checkpoint
    local blockPtr = statsManager.valuesPtr + (blockIndex * STATS_COUNT * STAT_SIZE_BYTES)
    local statsValues = {}
    for i = 1,STATS_COUNT do
        local value = safeRead("stats value " .. i, readShortInteger, blockPtr, true)
        statsValues[i] = value
        blockPtr = blockPtr + 2  -- each value is 2 bytes
    end
    return statsValues
end

function getChallenges(base)
    local headPtr = base + 0xD617C0 + 0x08
    local nodePtr = safeRead("head pointer", readPointer, headPtr)
    local count = 0
    while nodePtr ~= headPtr do
        local dataPtr = safeRead("data pointer", readPointer, nodePtr + 0x0C)
        local completed = safeRead("completed", readByte, dataPtr + 0x98, true)
        if completed ~= 0 then count = count + 1 end
        nodePtr = safeRead("next node pointer", readPointer, nodePtr)
    end
    return count
end

function getRawScore(statsManager, statsValues)
    local entryPtr = statsManager.entryBeginPtr
    local sum = 0
    local num_entries = 0
    while entryPtr ~= statsManager.entryEndPtr do
        local descriptorPtr = safeRead("entry descriptor", readPointer, entryPtr + 0x04)
        if descriptorPtr ~= 0 then
            local index = safeRead("entry index", readInteger, descriptorPtr + 0x34, true)
            if index < 0 or index >= STATS_COUNT then error("Index out of range: " .. index) end
            local multiplier = safeRead("entry multiplier", readInteger, descriptorPtr + 0x3C, true)
            local value = statsValues[index + 1]
            sum = sum + value * multiplier
        end
        num_entries = num_entries + 1
        entryPtr = entryPtr + 0x08
        if num_entries >= STATS_COUNT then error("Too many entries") end
    end
    return sum
end

local DIFFICULTY_SCALE = {100, 125, 150, 200, 250}

function getScore(rawScore, difficulty, challenges)
    return (rawScore * (DIFFICULTY_SCALE[difficulty + 1] + 5 * challenges) + 50) // 100
end

function main()
    local base = getAddressSafe("HMA.exe")
    if base == nil then error("HMA.exe not found") end
    print(string.format("Base: %08X", base))
    local difficulty = getDifficulty(base)
    print("Difficulty: " .. difficulty)
    local level = getLevel(base)
    print("Level: " .. level)
    local checkpoint = getCheckpoint(base)
    print("Checkpoint: " .. checkpoint)
    local challenges = getChallenges(base)
    print("Challenges: " .. challenges)
    if checkpoint >= 0 then
        local statsManager = getStatsManager(base)
        local statsValues = getStatsValues(statsManager, level, checkpoint)
        print("Stats values: " .. table.concat(statsValues, ", "))
        local rawScore = getRawScore(statsManager, statsValues)
        print("Raw score: " .. rawScore)
        local score = getScore(rawScore, difficulty, challenges)
        print("Score: " .. score)
    end
end

main()
