-- lua script for documenting and testing pointers

local CHECKPOINTS_PER_LEVEL = 13
local STATS_COUNT           = 100
local STAT_SIZE_BYTES       = 2

function safeRead(name, fn, addr, ...)
    local val = fn(addr, ...)
    if val == nil then error("Unable to read " .. name, 2) end
    return val
end

function getCheckpoint(base, level)
    local info = safeRead("info", readPointer, base + 0xE21580 + 0x28)
    if info == 0 then
        -- no level loaded
        return -1
    end
    local entryPtr = safeRead("first entry pointer", readPointer, info + 0x0C)
    local entryEnd = safeRead("end entry pointer", readPointer, info + 0x10)
    local key = safeRead("current checkpoint key", readInteger, info + 0x50)
    if key == 0 then
       -- no level loaded
       return -1
    end
    local checkpoint = 0
    while entryPtr ~= entryEnd do
        local entryKey = safeRead("checkpoint key", readInteger, entryPtr)
        if entryKey == key then
            return checkpoint
        end
        entryPtr = entryPtr + 8  -- each entry is 8 bytes
        checkpoint = checkpoint + 1
        if checkpoint >= CHECKPOINTS_PER_LEVEL then error("Checkpoint table too large") end
    end
    error("Unable to find current checkpoint key")
end

function getStatsValues(base, level, checkpoint)
    local statsBase = safeRead("stats base pointer", readPointer, base + 0xD61710 + 0x28)
    local blockIndex = (level * CHECKPOINTS_PER_LEVEL) + checkpoint
    local blockPtr = statsBase + (blockIndex * STATS_COUNT * STAT_SIZE_BYTES)
    local statsValues = {}
    for i = 1,100 do
        local value = safeRead("stats entry " .. i, readShortInteger, blockPtr)
        statsValues[i] = value
        blockPtr = blockPtr + 2
    end
    return statsValues
end

function getChallenges(base)
    local sentinel = base + 0xD617C0 + 0x08
    local node = safeRead("first node pointer", readPointer, sentinel)
    local count = 0
    while node ~= sentinel do
        local data = safeRead("data pointer", readPointer, node + 0x0C)
        local completed = safeRead("completed", readByte, data + 0x98)
        if completed ~= 0 then
            count = count + 1
        end
        node = safeRead("next node pointer", readPointer, node)
    end
    return count
end

function getRawScore(base, statsValues)
    local keyPtr = safeRead("first key pointer", readPointer, base + 0xD61710 + 0x04)
    local keyEnd = safeRead("end key pointer", readPointer, base + 0xD61710 + 0x08)
    local sum = 0
    while keyPtr ~= keyEnd do
        local descriptor = safeRead("key descriptor", readPointer, keyPtr + 0x04)
        if descriptor ~= 0 then
            local index = safeRead("key index", readInteger, descriptor + 0x34, true)
            if index < 0 or index >= STATS_COUNT then
                error("Index out of range: " .. index)
            end
            local multiplier = safeRead("key multiplier", readInteger, descriptor + 0x3C, true)
            local value = statsValues[index + 1]
            sum = sum + value * multiplier
        end
        keyPtr = keyPtr + 0x08
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

    local difficulty = safeRead("difficulty", readInteger, base + 0xD58D04, true)
    print("Difficulty: " .. difficulty)

    local level = safeRead("level", readInteger, base + 0xE21394, true)
    print("Level: " .. level)

    local challenges = getChallenges(base)
    print("Challenges: " .. challenges)

    local checkpoint = getCheckpoint(base, level)
    print("Checkpoint: " .. checkpoint)

    if checkpoint >= 0 then
        local statsValues = getStatsValues(base, level, checkpoint)
        print("Stats: " .. table.concat(statsValues, ", "))

        local rawScore = getRawScore(base, statsValues)
        print("Raw score: " .. rawScore)

        local score = getScore(rawScore, difficulty, challenges)
        print("Score: " .. score)
    end
end

main()
