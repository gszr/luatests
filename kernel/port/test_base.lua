print "TESTING BASE..."

print("creating a temp file...")
local tmpname = os.tmpname()
print("temp file = " .. tmpname)

print("setting output...")
local out = io.output(tmpname)
print("io.output = " .. type(out))

local prog = [[
a = 2
]]

print("writing to " .. tmpname .. "...")
out:write(prog)

print("closing file...")
print("close status = " .. tostring(out.close()))

dofile(tmpname)
