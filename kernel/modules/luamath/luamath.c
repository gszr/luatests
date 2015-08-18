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

static int math_random (lua_State *L) {
  lua_Integer low, up;
  //XXX
  //double r = (double)l_rand() * (1.0 / ((double)L_RANDMAX + 1.0));
  long r = random();
  switch (lua_gettop(L)) {  /* check number of arguments */
    case 0: {  /* no arguments */
      lua_pushnumber(L, (lua_Number)r);  /* Number between 0 and 1 */
      return 1;
    }
    case 1: {  /* only upper limit */
      low = 1;
      up = luaL_checkinteger(L, 1);
      break;
    }
    case 2: {  /* lower and upper limits */
      low = luaL_checkinteger(L, 1);
      up = luaL_checkinteger(L, 2);
      break;
    }
    default: return luaL_error(L, "wrong number of arguments");
  }
  /* random integer in the interval [low, up] */
  luaL_argcheck(L, low <= up, 1, "interval is empty"); 
  //XXX
  //luaL_argcheck(L, low >= 0 || up <= LUA_MAXINTEGER + low, 1,
  //                 "interval too large");
  r *= (up - low) + 1;
  lua_pushinteger(L, (lua_Integer)r + low);
  return 1;
}

static int math_sin (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1));
  return 1;
} 

static int math_cos (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1));
  return 1;
}
 

static int math_randomseed (lua_State *L) {
  //XXX seed random()?
  srandom((unsigned int)(lua_Integer)luaL_checknumber(L, 1));
  (void)random(); /* discard first value to avoid undesirable correlations */
  return 0;
}

static int math_fmod (lua_State *L) {
    lua_Integer d = lua_tointeger(L, 2);
    if ((lua_Unsigned)d + 1u <= 1u) {  /* special cases: -1 or 0 */
      luaL_argcheck(L, d != 0, 2, "zero");
      lua_pushinteger(L, 0);  /* avoid overflow with 0x80000... / -1 */
    }
    else
      lua_pushinteger(L, lua_tointeger(L, 1) % d);
  	return 1;
} 

static int
luaopen_math(lua_State *L)
{
	const luaL_Reg math_lib[] = {
		{"random", math_random},
		{"randomseed", math_randomseed},
		{"fmod", math_fmod},
		{"sin", math_sin},
		{"cos", math_cos},
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
