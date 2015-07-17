local os = require 'os'

print(os)

for k, v in pairs(os) do print(k) end

print("os.time() = " .. os.time())

