
print("os.time() = " .. os.time())
print("os.clock() = " .. os.clock())
print("os.tmpname() 1 = " .. os.tmpname())
print("os.tmpname() 2 = " .. os.tmpname())

local filename = os.tmpname()
print("os.tmpname() 3 = " .. filename)

print("os.remove() = " .. tostring(os.remove(filename)))
