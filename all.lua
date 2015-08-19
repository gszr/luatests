-- XXX Kernel Lua: reporting shebang as a syntax error
--#!../lua
-- $Id: all.lua,v 1.91 2014/12/26 17:20:53 roberto Exp $

local version = "Lua 5.3"
if _VERSION ~= version then
  io.stderr:write("\nThis test suite is for ", version, ", not for ", _VERSION,
    "\nExiting tests\n")
  return
end

-- XXX Kernel Lua: globals are erased below
local _KERNEL = _KERNEL

-- next variables control the execution of some tests
-- true means no test (so an undefined variable does not skip a test)
-- defaults are for Linux; test everything.
-- Make true to avoid long or memory consuming tests
_soft = rawget(_G, "_soft") or false
-- Make true to avoid non-portable tests
_port = rawget(_G, "_port") or false
-- Make true to avoid messages about tests not performed
_nomsg = rawget(_G, "_nomsg") or false


local usertests = rawget(_G, "_U")

if usertests then
  -- tests for sissies ;)  Avoid problems
  _soft = true
  _port = true
  _nomsg = true
end

-- XXX Kernel Lua: libs are loaded only once (see preload.lua)
-- 				   to avoid the bug on require
if not _KERNEL then
-- tests should require debug when needed
debug = nil
end

if usertests then
  T = nil    -- no "internal" tests for user tests
else
  T = rawget(_G, "T")  -- avoid problems with 'strict' module
end

math.randomseed(0)

--[=[
  example of a long [comment],
  [[spanning several [lines]]]

]=]

print("current path:\n****" .. package.path .. "****\n")


local initclock = os.clock()
local lastclock = initclock
local walltime = os.time()

local collectgarbage = collectgarbage

do   -- (

-- track messages for tests not performed
local msgs = {}
function Message (m)
  if not _nomsg then
    print(m)
    msgs[#msgs+1] = string.sub(m, 3, -3)
  end
end

-- XXX Kernel Lua: no locale
if not _KERNEL then
assert(os.setlocale"C")
end

local T,print,format,write,assert,type,unpack,floor =
      T,print,string.format,io.write,assert,type,table.unpack,math.floor

-- use K for 1000 and M for 1000000 (not 2^10 -- 2^20)
local function F (m)
  local function round (m)
    -- XXX Kernel Lua: no float
	if not _KERNEL then
	eval[[m = m + 0.04999
    return format("%.1f", m)]]      -- keep one decimal digit
	else
	return m
	end
  end
  if m < 1000 then return m
  else
    m = m / 1000
    if m < 1000 then return round(m).."K"
    else
      return round(m/1000).."M"
    end
  end
end

local showmem
if not T then
  local max = 0
  showmem = function ()
    local m = collectgarbage("count") * 1024
    max = (m > max) and m or max
    print(format("    ---- total memory: %s, max memory: %s ----\n",
          F(m), F(max)))
  end
else
  showmem = function ()
    T.checkmemory()
    local total, numblocks, maxmem = T.totalmem()
    local count = collectgarbage("count")
	-- XXX Kernel Lua: no float
	if not _KERNEL then
    print(format(
      "\n    ---- total memory: %s (%.0fK), max use: %s,  blocks: %d\n",
      F(total), count, F(maxmem), numblocks))
	else
    print(format(
      "\n    ---- total memory: %s (%sK), max use: %s,  blocks: %d\n",
      F(total), count, F(maxmem), numblocks))
	end
    print(format("\t(strings:  %d, tables: %d, functions: %d, "..
                 "\n\tudata: %d, threads: %d)",
                 T.totalmem"string", T.totalmem"table", T.totalmem"function",
                 T.totalmem"userdata", T.totalmem"thread"))
  end
end


--
-- redefine dofile to run files through dump/undump
--
local function report (n) print("\n***** FILE '"..n.."'*****") end
local olddofile = dofile
local dofile = function (n, strip)
  showmem()
  local c = os.clock()
  -- XXX Kernel Lua: no float
  if not _KERNEL then
  print(string.format("time: %g (+%g)", c - initclock, c - lastclock))
  else
  print(string.format("time: %s (+%s)", tostring(c - initclock), tostring(c - lastclock)))
  end
  lastclock = c
  report(n)
  local f = assert(loadfile(n))
  local b = string.dump(f, strip)
  f = assert(load(b))
  return f()
end

-- XXX Kernel Lua: main.lua assumes a shell
if not _KERNEL then
dofile('main.lua')
end

do
  local next, setmetatable, stderr = next, setmetatable, io.stderr
  -- track collections
  local mt = {}
  -- each time a table is collected, remark it for finalization
  -- on next cycle
  mt.__gc = function (o)
     stderr:write'.'    -- mark progress
     local n = setmetatable(o, mt)   -- remark it
   end
   local n = setmetatable({}, mt)    -- create object
end

report"gc.lua"
local f = assert(loadfile('gc.lua'))
f()

dofile('db.lua')
assert(dofile('calls.lua') == deep and deep)
olddofile('strings.lua')
olddofile('literals.lua')
dofile('tpack.lua')
assert(dofile('attrib.lua') == 27)

assert(dofile('locals.lua') == 5)
dofile('constructs.lua')
dofile('code.lua', true)
if not _G._soft then
  report('big.lua')
  local f = coroutine.wrap(assert(loadfile('big.lua')))
  assert(f() == 'b')
  assert(f() == 'a')
end
dofile('nextvar.lua')
dofile('pm.lua')
dofile('utf8.lua')
dofile('api.lua')
assert(dofile('events.lua') == 12)
dofile('vararg.lua')
dofile('closure.lua')
dofile('coroutine.lua')
dofile('goto.lua', true)
dofile('errors.lua')
-- XXX Kernel Lua: math.lua not ported
if not _KERNEL then
dofile('math.lua')
end
dofile('sort.lua', true)
dofile('bitwise.lua')
assert(dofile('verybig.lua', true) == 10); collectgarbage()
-- XXX Kernel Lua: files.lua not ported
if not _KERNEL then
dofile('files.lua')
end

if #msgs > 0 then
  print("\ntests not performed:")
  for i=1,#msgs do
    print(msgs[i])
  end
  print()
end

-- no test module should define 'debug'
-- XXX Kernel Lua: debug was not erased (see note above)
if not _KERNEL then
assert(debug == nil)
end

local debug = require "debug"

print(string.format("%d-bit integers, %d-bit floats",
        string.packsize("j") * 8, string.packsize("n") * 8))

debug.sethook(function (a) assert(type(a) == 'string') end, "cr")

-- to survive outside block
_G.showmem = showmem

end   --)

local _G, showmem, print, format, clock, time, assert, open =
      _G, showmem, print, string.format, os.clock, os.time, assert, io.open

-- file with time of last performed test
local fname = T and "time-debug.txt" or "time.txt"
local lasttime

if not usertests then
  -- open file with time of last performed test
  local f = io.open(fname)
  if f then
    lasttime = assert(tonumber(f:read'a'))
    f:close();
  else   -- no such file; assume it is recording time for first time
    lasttime = nil
  end
end

-- erase (almost) all globals
print('cleaning all!!!!')
for n in pairs(_G) do
  if not ({___Glob = 1, tostring = 1})[n] then
    _G[n] = nil
  end
end


collectgarbage()
collectgarbage()
collectgarbage()
collectgarbage()
collectgarbage()
collectgarbage();showmem()

local clocktime = clock() - initclock
walltime = time() - walltime

-- XXX Kernel Lua: no float
if not _KERNEL then
print(format("\n\ntotal time: %.2fs (wall time: %ds)\n", clocktime, walltime))
else
print(format("\n\ntotal time: %ss (wall time: %ss)\n", tostring(clocktime), tostring(walltime)))
end

if not usertests then
  lasttime = lasttime or clocktime    -- if no last time, ignore difference
  -- check whether current test time differs more than 5% from last time
  -- XXX Kernel Lua: no float
  local diff, tolerance
  if not _KERNEL then
  eval[[diff = (clocktime - lasttime) / clocktime
  tolerance = 0.05    -- 5%]]
  else
  diff = 100 - (lasttime * 100) / clocktime
  tolerance = 5
  end

  if (diff >= tolerance or diff <= -tolerance) then
  -- XXX Kernel Lua: no float
  if not _KERNEL then
    print(format("WARNING: time difference from previous test: %+.1f%%",
                  diff * 100))
  else
    print(format("WARNING: time difference from previous test: %s%%",
                  tostring(diff)))
  end
  end
  assert(open(fname, "w")):write(clocktime):close()
end

print("final OK !!!")

