local ctypes = require("mempeep.ctypes")
local hma = require("hma")

local out = io.open("hma_structs.hpp", "w")
if out then
  out:write([[#pragma once

#include <array>
#include <cstdint>
#include <mempeep/descriptors.hpp>
#include <optional>
#include <string>
#include <vector>

using namespace mempeep;
]])
  ctypes.native_struct_cdecls(hma.Game, out)
  ctypes.mempeep_struct_cdecls(hma.Game, out)
  out:close()
end
