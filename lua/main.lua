local ctypes = require("mempeep.ctypes")
local hbm = require("hbm")
local hma = require("hma")

do
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

  local out2 = io.open("hitman_absolution/structs.txt", "w")
  if out2 then
    ctypes.remote_struct_cdecls(hma.Game, 4, out2)
    out2:close()
  end
end

do
  local out = io.open("hitman_blood_money/structs.hpp", "w")
  if out then
    out:write([[#pragma once

#include <array>
#include <cstdint>
#include <mempeep/descriptors.hpp>

using namespace mempeep;

namespace hitman_blood_money::structs {

]])
    ctypes.native_struct_cdecls(hbm.Game, "", out)
      out:write([[
}
]])
    out:close()
  end

  local out2 = io.open("hitman_blood_money/structs.txt", "w")
  if out2 then
    ctypes.remote_struct_cdecls(hbm.Game, 4, out2)
    out2:close()
  end
end
