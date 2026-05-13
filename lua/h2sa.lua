M = {}

local d = require("mempeep.descriptors")

local SmallString = d.Struct("SmallString", {
    d.Field(d.Ref(d.ZString(0x100)), "text"),
    d.Skip(0x7C),  -- inline buffer for strings <= 0x7C chars
})

local SceneManager = d.Struct("SceneManager", {
    d.Seek(0xBB7),
    d.Field(SmallString, "scene_name"),
})

local Engine = d.Struct("Engine", {
    d.Seek(0x98),
    d.Field(d.Ref(SceneManager), "scene_manager"),
})

M.Game = d.Struct("Game", {
    d.Seek(0x2A6C5C),
    d.Field(d.Ref(Engine), "engine"),
})

M.mission_scene_names = {
    "SCENES\\C0-1\\C0-1__MAIN.gms",
    "SCENES\\C1-1\\C1-1__MAIN.gms",
    "SCENES\\C2-1\\C2-1__MAIN.gms",
    "SCENES\\C2-2\\C2-2__MAIN.gms",
    "SCENES\\C2-3\\C2-3__MAIN.gms",
    "SCENES\\C2-4\\C2-4__MAIN.gms",
    "SCENES\\C3-1\\C3-1__MAIN.gms",
    "SCENES\\C3-2a\\C3-2a__MAIN.gms",
    "SCENES\\C3-2b\\C3-2b__MAIN.gms",
    "SCENES\\C3-3\\C3-3__MAIN.gms",
    "SCENES\\C4-1\\C4-1__MAIN.gms",
    "SCENES\\C4-2\\C4-2__MAIN.gms",
    "SCENES\\C4-3\\C4-3__MAIN.gms",
    "SCENES\\C5-1\\C5-1__MAIN.gms",
    "SCENES\\C5-2\\C5-2__MAIN.gms",
    "SCENES\\C5-3\\C5-3__MAIN.gms",
    "SCENES\\C6-1\\C6-1__MAIN.gms",
    "SCENES\\C6-2\\C6-2__MAIN.gms",
    "SCENES\\C6-3\\C6-3__MAIN.gms",
    "SCENES\\C7-1\\C7-1__MAIN.gms",
    "SCENES\\C8-1\\C8-1__MAIN.gms",
}

return M
