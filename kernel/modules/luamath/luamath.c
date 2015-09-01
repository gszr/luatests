/*
 * See copyright notice in LICENSE
 */

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
  r = r % (up - low) + low;
  lua_pushinteger(L, (lua_Integer)r);
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

static int math_min (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  int imin = 1;  /* index of current minimum value */
  int i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  for (i = 2; i <= n; i++) {
    if (lua_compare(L, i, imin, LUA_OPLT))
      imin = i;
  }
  lua_pushvalue(L, imin);
  return 1;
} 

static int math_max (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  int imax = 1;  /* index of current maximum value */
  int i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  for (i = 2; i <= n; i++) {
    if (lua_compare(L, imax, i, LUA_OPLT))
      imax = i;
  }
  lua_pushvalue(L, imax);
  return 1;
} 

static int math_type (lua_State *L) {
  if (lua_type(L, 1) == LUA_TNUMBER) {
      if (lua_isinteger(L, 1))
        lua_pushliteral(L, "integer"); 
      else
        lua_pushliteral(L, "float"); 
  }
  else {
    luaL_checkany(L, 1);
    lua_pushnil(L);
  }
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
		{"min", math_min},
		{"max", math_max},
		{"type", math_type},
		{NULL, NULL}
	};

	luaL_newlib(L, math_lib);
	//XXX __builtin_huge_val()?
 	lua_pushnumber(L, (lua_Number)__builtin_huge_val());
 	lua_setfield(L, -2, "huge");
  	lua_pushinteger(L, LUA_MAXINTEGER);
  	lua_setfield(L, -2, "maxinteger");
  	lua_pushinteger(L, LUA_MININTEGER);
  	lua_setfield(L, -2, "mininteger");

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
