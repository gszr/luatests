/* Lua OS module */

#include <sys/time.h>
#include <sys/lua.h>
#ifdef _MODULE
#include <sys/module.h>
#endif

#include <lua.h>
#include <lauxlib.h>

#include "../iolib/iolib.h"
#include "../mkstemp/mkstemp.h"

#define LUA_TMPNAMTEMPLATE "/tmp/lua_XXXXXX"
#define LUA_TMPNAMEBUFSIZE 32

#define lua_tmpnam(b,e) { \
   		strcpy(b, LUA_TMPNAMTEMPLATE); \
		e = mkstemp(b); \
		if (e != -1) kclose(e); \
		e = (e == -1); }

#define l_timet			lua_Integer
#define l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))

#ifdef _MODULE
MODULE(MODULE_CLASS_MISC, luaos, "lua");

//XXX loslib
static int
os_time (lua_State *L) {
	time_t t;
	struct bintime bt;
  	if (lua_isnoneornil(L, 1)) {  /* called without args? */
		bintime(&bt);
    	t = bt.sec;  /* get current time */
  	} else {
		//XXX check it out later
		#if 0 
		struct tm ts;
    	luaL_checktype(L, 1, LUA_TTABLE);
    	lua_settop(L, 1);  /* make sure table is at the top */
    	ts.tm_sec = getfield(L, "sec", 0);
    	ts.tm_min = getfield(L, "min", 0);
    	ts.tm_hour = getfield(L, "hour", 12);
    	ts.tm_mday = getfield(L, "day", -1);
    	ts.tm_mon = getfield(L, "month", -1) - 1;
    	ts.tm_year = getfield(L, "year", -1) - 1900;
    	ts.tm_isdst = getboolfield(L, "isdst");
    	t = mktime(&ts);
		#endif
 	}	
	if (t != (time_t)(l_timet)t)
    	luaL_error(L, "time result cannot be represented in this Lua instalation");
  	else if (t == (time_t)(-1))
    	lua_pushnil(L);
  	else
   		l_pushtime(L, t);
	return 1;
} 

static int
os_utime(lua_State *L)
{
	struct timeval tv;
	microtime(&tv);
	lua_pushinteger(L, tv.tv_usec);
	return 1;
}

static int
os_clock(lua_State *L)
{
	return os_utime(L);
}

static int
/* loslib.c */
os_tmpname(lua_State *L) {
	char buff[LUA_TMPNAMEBUFSIZE];
	int err;
	lua_tmpnam(buff, err);
	if (err)
		return luaL_error(L, "unable to generate a unique filename");
	lua_pushstring(L, buff);
	return 1;
}

static int
luaopen_os(lua_State *L)
{
	const luaL_Reg os_lib[] = {
		{"time",  os_time},
		{"clock", os_clock},
		{"tmpname", os_tmpname},
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
