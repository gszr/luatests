local math = require 'math'

print(math)
for k, v in pairs(math) do print(k) end

print("math.ipow(2, 3) = " .. math.ipow(2, 3))
print("math.random() = " .. math.random())
print("math.fmod(123, 44) = ".. math.fmod(15123, 214))
