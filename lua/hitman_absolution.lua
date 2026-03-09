memreader = require("memreader")
T = memreader.T

local hitman_absolution = {}

hitman_absolution.structs = {
    Game = {
        size = 0x7FFFFFFF, -- placeholder
        fields = {
            {name = "difficulty", offset = 0xD58C60 + 0x10 + 0x94, type_def = T.i32},
            {name = "level", offset = 0xE21394, type_def = T.i32},
            {name = "checkpoints_manager", offset = 0xE21580, type_def = T.struct("CheckpointsManager")}
        }
    },
    CheckpointsManager = {
        size = 0x2C, -- unknown
        fields = {
            {name = "checkpoints", offset = 0x28, type_def = T.ptr(T.struct("Checkpoints"))}
        }
    },
    Checkpoints = {
        size = 0x54, -- unknown
        fields = {
            {name = "checkpoint", offset = 0x0C, type_def = T.vector(T.struct("Checkpoint"))},
            {name = "current_key", offset = 0x50, type_def = T.i32}
        }
    },
    Checkpoint = {
        size = 0x08,
        fields = {
            {name = "key", offset = 0x00, type_def = T.i32}, -- could be pointer
            {name = "_unknown", offset = 0x04, type_def = T.i32} -- could be pointer
        }
    }
}

hitman_absolution.get_current_checkpoint_index = function(checkpoints)
    if checkpoints.current_key == 0 then
        return -1
    end -- no checkpoint loaded
    for i, checkpoint in ipairs(checkpoints.checkpoint) do
        if checkpoint.key == checkpoints.current_key then
            return i - 1
        end
    end
    error("Unable to find checkpoint key")
end

return hitman_absolution
