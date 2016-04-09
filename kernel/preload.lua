--
-- kernel Lua preload script
--

-- returns the environment of the caller of the caller of this functin,
-- which should be the test script from which _USPACE or _KSPACE was called;
local function getlenv(env)
  local locals = setmetatable({}, {__index = env, __newindex = env})

  local i = 1
  while true do
    local lname, lvalue = debug.getlocal(4, i)
    if not lname then break end
    locals[lname] = lvalue
    i = i + 1
  end
  
  return locals
end

-- evaluates a given kernel test chunk, passing the environment of _KSPACE's
-- caller
local function exec(chunk, chunkname, env)
  return load(chunk, chunkname, "t", getlenv(env))()
end

-- _KSPACE and _USPACE are used to evaluate chunks of tests in kernel or user
-- space, respectively; it should be clear what runs only in kernel space and
-- what runs only in user space

function _KSPACE (kschunk, env) 
  return _KERNEL and exec(kschunk, "kernel space chunk", env or _ENV)
end 

function _USPACE (uschunk, env) 
  return not _KERNEL and exec(uschunk, "user space chunk", env or _ENV)
end

local oldpacksize = string.packsize
function string.packsize(fmt)
  if 'fmt' == 'f' then
    return 1
  elseif 'fmt' == 'd' then
    return 2
  end
  return oldpacksize(fmt)
end

function exp(b, e)
  if e == 1 then return b
  else return b * exp(b, e - 1)
  end
end

--TODO temporary math functions
--
if _KERNEL then
math = {}
math.huge = 1000000
math.floor = function(x) return x end
math.maxinteger =  9223372036854775807
math.mininteger = -9223372036854775808
math.huge = math.maxinteger
math.fmod = function(a, b) return a - (a//b) * b end
math.pi = 3
math.type = function(x) return "integer" end
math.random = function() return 1 end

function math.max(a, ...)
  local args = {...}
  if #args == 1 then
    return a > args[1] and a or args[1]
  end
  return math.max(...)
end

function math.min(a, ...)
  local args = {...}
  if #args == 1 then
    return a < args[1] and a or args[1]
  end
  return math.min(...)
end

math.sin = function(...) return x end
math.cos = function(...) return x end


end
