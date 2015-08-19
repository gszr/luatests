-- kernel Lua preload script

_KERNEL	= true

os   = require'os'
io 	 = require'io'
math = require'math'
base = require'base'

dofile = base.dofile
_require = require

function require (module)
	return _G[module]
end

function eval (chunk) 
	return load(chunk)() 
end


--------------------------------------------------------
-- taken from http://www.lua.org/pil/14.1.html
function setfield (f, v)
	local t = _G    -- start with the table of globals
	for w, d in string.gmatch(f, "([%w_]+)(.?)") do
		if d == "." then        -- not last field?
			t[w] = t[w] or {}   -- create table if absent
			t = t[w]            -- get the table
		else                    -- last field
			t[w] = v            -- do the assignment
		end
	end
end
--------------------------------------------------------

setfield("package.preload", {})
setfield("package.loaded", {})

package.loaded.io = io
package.loaded.os = os
package.loaded.math = math
package.loaded.string = string
package.loaded.table = table
package.loaded.debug = debug
package.loaded.coroutine = coroutine
package.loaded.utf8 = utf8
package.loaded._G = _G
package.loaded.package = package
