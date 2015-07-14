-- kernel Lua preload script

_KERNEL	= true

eval 	= function(chunk) return load(chunk)() end


