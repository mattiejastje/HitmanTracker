local ctypes = require("mempeep.ctypes")
local hbm = require("hbm")
local hma = require("hma")
local hc47 = require("hc47")

--- Write native C++ struct declarations to a .hpp file and remote layout to a .txt file.
-- @param dir output directory (no trailing slash)
-- @param descs root descriptor passed to ctypes functions
-- @param namespace C++ namespace name wrapping the declarations
local function write_structs(dir, descs, namespace)
  local hpp, hpp_err = io.open(dir .. "/structs.hpp", "w")
  assert(hpp, "failed to open " .. dir .. "/structs.hpp: " .. tostring(hpp_err))
  hpp:write("#pragma once\n\n")
  ctypes.write_native_includes(descs, hpp)
  hpp:write("\nusing namespace mempeep;\n\n")
  hpp:write("namespace " .. namespace .. " {\n\n")
  ctypes.native_struct_cdecls(descs, "", hpp)
  hpp:write("}  // namespace " .. namespace .. "\n")
  hpp:close()

  local txt, txt_err = io.open(dir .. "/structs.txt", "w")
  assert(txt, "failed to open " .. dir .. "/structs.txt: " .. tostring(txt_err))
  ctypes.remote_struct_cdecls(descs, 4, txt)
  txt:close()
end

write_structs("hitman_absolution", { hma.Game }, "hitman_absolution::structs")
write_structs("hitman_blood_money", { hbm.Game }, "hitman_blood_money::structs")
write_structs("hitman_codename_47", { hc47.HitmanDlc }, "hitman_codename_47::structs")
