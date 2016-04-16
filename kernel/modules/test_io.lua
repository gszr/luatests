
print("creating a temp file...")
local tmpname = os.tmpname()
print("temp file = " .. tmpname)

print("setting output...")
local out = io.output(tmpname)
print("io.output = " .. type(out))

print("writing to " .. tmpname .. "...")
print(out:write("Hello, NetBSD world!\nThis is a test\n"))

print("closing file...")
print("close status = " .. tostring(out.close()))

print("setting input...")
local inp = io.input(tmpname)

print("reading all of " .. tmpname .. "...")
local str = inp:read("all")
print("file contents: " .. str)
inp:close()

local inp = io.input(tmpname)
print("reading first line of " .. tmpname .. "...")
local str = inp:read("line")
print("first line: " .. str)
inp:close()

local inp = io.input(tmpname)
print("reading first 5 chars of " .. tmpname .. "...")
local str = inp:read(5)
print("first 5 chars: " .. str)
inp:close()

io.output(tmpname)
io.close()

local f = io.open(tmpname, 'r')
local s = f:read('all')
f:close()
print(s)
