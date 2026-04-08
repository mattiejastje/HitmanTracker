local ctypes = require("mempeep.ctypes")
local hma = require("hma")

local out = io.open("hitman_absolution/structs.hpp", "w")
if out then
  out:write([[#pragma once

#include <array>
#include <cstdint>
#include <mempeep/descriptors.hpp>
#include <optional>
#include <string>
#include <vector>

using namespace mempeep;

namespace hitman_absolution::structs {

]])
  ctypes.native_struct_cdecls(hma.Game, "", out)
    out:write([[
}
]])
  out:close()
end
