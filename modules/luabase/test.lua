local os = require 'os'
local io = require 'io'
local base = require 'base'

print(base)

for k, v in pairs(base) do print(k) end

print("creating a temp file...")
local tmpname = os.tmpname()
print("temp file = " .. tmpname)

print("setting output...")
local out = io.output(tmpname)
print("io.output = " .. type(out))

local prog = [[
print'Hello, NetBSD world! Testing dofile!'
]]

print("writing to " .. tmpname .. "...")
out:write(prog)

print("closing file...")
print("close status = " .. tostring(out.close()))

base.dofile(tmpname)
