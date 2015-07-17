/* Lua OS module */

#include <sys/time.h>
#include <sys/lua.h>
#ifdef _MODULE
#include <sys/module.h>
#endif

#include <lua.h>
#include <lauxlib.h>

#ifdef _MODULE
MODULE(MODULE_CLASS_MISC, luaos, "lua");

static int 
os_time(lua_State *L)
{
	struct bintime bt;
	bintime(&bt);
	lua_pushinteger(L, bt.sec);
	return 1;
}

static int
luaopen_os(lua_State *L)
{
	const luaL_Reg os_lib[] = {
		{ "time",  os_time},
		{NULL, NULL}
	};

	luaL_newlib(L, os_lib);

	return 1;
}

static int
luaos_modcmd(modcmd_t cmd, void *opaque)
{
	int error;

	switch (cmd) {
	case MODULE_CMD_INIT:
		error = klua_mod_register("os", luaopen_os);
		break;
	case MODULE_CMD_FINI:
		error = klua_mod_unregister("os");
		break;
	default:
		error = ENOTTY;
	}
	return error;
}
#endif
