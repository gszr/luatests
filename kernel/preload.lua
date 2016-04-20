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

-- _KBUG is used to mark chunks that expose bugs in kernel Lua
-- perhaps it could do something useful -- e.g., save the chunk in a 
-- file?
function _KBUG(chunk) end

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
  if fmt == 'f' then
    return 1
  elseif fmt == 'd' then
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
math.floor = function(x) return x end
math.pi    = 3

math.sin = function(...) return x end
math.cos = function(...) return x end
end

package = {}
package.loaded = {}
package.preload = {}

package.loaded.os      = os
package.loaded.io      = io
package.loaded.math    = math
package.loaded.debug   = debug
package.loaded.string  = string
package.loaded.package = package
package.loaded.table   = table

function require(mod)
  if package.loaded[mod] then
    return package.loaded[mod]
  elseif package.preload[mod] then
    return package.preload[mod]()
  end
  return modules[mod]
end


