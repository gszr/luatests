local string       = string
local debug        = debug
local print        = print
local load         = load
local setmetatable = setmetatable

--
-- kernel Lua preload script
--

-- returns the environment of the caller of the caller of
-- this functin, which should be the test script from which
-- _USPACE or _KSPACE was called;
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

-- evaluates a given kernel test chunk, passing the environment
-- of _KSPACE's caller
local function exec(chunk, chunkname, env)
  print(chunk)
  return load(chunk, chunkname, "t", getlenv(env))()
end

-- _KBUG is used to mark chunks that expose bugs in kernel Lua
-- perhaps it could do something useful -- e.g., save the chunk
-- in a file?
function _KBUG(chunk) end

-- _KSPACE and _USPACE are used to evaluate chunks of tests in
-- kernel or user space, respectively; it should be clear what
-- runs only in kernel space and what runs only in user space
function _KSPACE (kschunk, env)
  return _KERNEL and exec(kschunk, "kernel space chunk",
                          env or _ENV)
end 

function _USPACE (uschunk, env) 
  return not _KERNEL and exec(uschunk, "user space chunk",
                              env or _ENV)
end

local opcksz = string.packsize
function string.packsize(fmt)
  if fmt == 'f' then
    return 4
  elseif fmt == 'd' then
    return 8
  end
  return opcksz(fmt)
end

local ostrfmt = string.format
function string.format(fmt, ...)
  return ostrfmt(string.gsub(fmt, '%%[+%.%d]*[fg]', '%%d'), ...)
end

-- os.clock returns ms in kernel lua
local oclock = os.clock
function os.clock()
  local clock = oclock()
  return clock >= 1000 and clock / 1000 or clock
end

function exp(b, e)
  if e == 1 then return b
  else return b * exp(b, e - 1)
  end
end

-- dummies: math [[
math.floor = function(x) return x end
math.pi    = 3

math.sin = function(...) return x end
math.cos = function(...) return x end
-- ]]
-- dummies: os   [[
os.setlocale = function(arg) return arg end
-- ]]
-- dummies: io   [[
io.stdout = {}
io.stdout.print = function(_, ...) print(...) end
io.stdout.write = io.stdout.print
io.stderr = io.stdout
-- ]]

package = {}
package.path = "."
package.loaded = {}
package.preload = {}

package.loaded.os        = os
package.loaded.io        = io
package.loaded.math      = math
package.loaded.debug     = debug
package.loaded.string    = string
package.loaded.package   = package
package.loaded.coroutine = coroutine
package.loaded.table     = table

function require(mod)
  return package.loaded[mod] and package.loaded[mod] or
         package.preload[mod] and package.preload[mod]() or
         modules[mod]
end
