local os = require 'os'

print(os)

for k, v in pairs(os) do print(k) end

print("os.time() = " .. os.time())
print("os.clock() = " .. os.clock())
print("os.tmpname() 1 = " .. os.tmpname())
print("os.tmpname() 2 = " .. os.tmpname())
print("os.tmpname() 3 = " .. os.tmpname())
