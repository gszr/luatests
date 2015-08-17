/* Lua OS module */

#include <sys/lua.h>
#include <sys/time.h>
#include <sys/stdint.h>
#ifdef _MODULE
#include <sys/module.h>
#endif

#include <lua.h>
#include <lauxlib.h>

#ifdef _MODULE
MODULE(MODULE_CLASS_MISC, luamath, "lua");

static int 
math_ipow(lua_State *L)
{
	int b, e, r = 1;

	b = lua_tointeger(L, -2);
	e = lua_tointeger(L, -1);
	
	if (e < 0) {
		lua_pushinteger(L, 0);
		return 1;
	}

	while(e)
	{
		if (e & 1)
			r *= b;
		e >>= 1;
		b *= b;
	}

	lua_pushinteger(L, r);

	return 1;
}

static int
math_fmod(lua_State *L)
{
	int a, b;
	a = lua_tointeger(L, -2);
	b = lua_tointeger(L, -1);
    lua_pushinteger(L, a - (int)(a/b)*b);
	return 1;
}

static int
math_random(lua_State *L)
{
	lua_pushinteger(L, random());
	return 1;
}

static int
luaopen_math(lua_State *L)
{
	const luaL_Reg math_lib[] = {
		{"ipow",  math_ipow},
		{"random", math_random},
		{"fmod", math_fmod},
		{NULL, NULL}
	};

	luaL_newlib(L, math_lib);

	return 1;
}

static int
luamath_modcmd(modcmd_t cmd, void *opaque)
{
	int error;

	switch (cmd) {
	case MODULE_CMD_INIT:
		error = klua_mod_register("math", luaopen_math);
		break;
	case MODULE_CMD_FINI:
		error = klua_mod_unregister("math");
		break;
	default:
		error = ENOTTY;
	}
	return error;
}
#endif
