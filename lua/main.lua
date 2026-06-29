local ctypes = require("mempeep.ctypes")
local hbm = require("hbm")
local hma = require("hma")
local hc47 = require("hc47")
local h2sa = require("h2sa")
local hcon = require("hcon")

--- Write native C++ struct declarations to a .hpp file
-- @param dir output directory (no trailing slash)
-- @param descs root descriptor passed to ctypes functions
-- @param namespace C++ namespace name wrapping the declarations
local function write_structs_cpp(dir, descs, namespace)
  local hpp, hpp_err = io.open(dir .. "/structs.hpp", "w")
  assert(hpp, "failed to open " .. dir .. "/structs.hpp: " .. tostring(hpp_err))
  hpp:write("#pragma once\n\n")
  ctypes.write_native_includes(descs, hpp)
  hpp:write("\nusing namespace mempeep;\n\n")
  hpp:write("namespace " .. namespace .. " {\n\n")
  ctypes.native_struct_cdecls(descs, 4, "", hpp)
  hpp:write("}  // namespace " .. namespace .. "\n")
  hpp:close()
end

--- Write remote layout to a .txt file
-- @param dir output directory (no trailing slash)
-- @param descs root descriptor passed to ctypes functions
local function write_structs_txt(dir, descs)
  local txt, txt_err = io.open(dir .. "/structs.txt", "w")
  assert(txt, "failed to open " .. dir .. "/structs.txt: " .. tostring(txt_err))
  ctypes.remote_struct_cdecls(descs, 4, txt)
  txt:close()
end

write_structs_txt("hitman_absolution", { hma.GameSteam, hma.GameGOG })
write_structs_cpp("hitman_absolution", { hma.GameSteam, hma.GameGOG }, "hitman_absolution::structs")
write_structs_txt("hitman_blood_money", { hbm.Game })
write_structs_cpp("hitman_blood_money", { hbm.Game }, "hitman_blood_money::structs")
write_structs_txt("hitman_codename_47", { hc47.HitmanDlc })
write_structs_cpp("hitman_codename_47", { hc47.HitmanDlc }, "hitman_codename_47::structs")
h2sa_structs = {
  h2sa.LevelControl,
  h2sa.PlayerEntity,
  h2sa.Player,
  h2sa.PropertyManagerRecord,
  h2sa.Game,
}
write_structs_txt("hitman2_silent_assassin", h2sa_structs)
write_structs_cpp("hitman2_silent_assassin", h2sa_structs, "hitman2_silent_assassin::structs")
hcon_structs = {
  h2sa.PropertyManagerRecord,
  hcon.HitmanContracts,
}
write_structs_txt("hitman_contracts", hcon_structs)
write_structs_cpp("hitman_contracts", hcon_structs, "hitman_contracts::structs")
