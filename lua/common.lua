local M = {}

M.add_fields = function(fields, new_fields)
  for _, item in ipairs(new_fields) do
    fields[#fields + 1] = item
  end
end

M.add_fields_if = function(cond, fields, new_fields)
  if cond then
    M.add_fields(fields, new_fields)
  end
end

return M
