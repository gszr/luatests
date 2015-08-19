/* Lua OS module */

#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <sys/lua.h>
#ifdef _MODULE
#include <sys/module.h>
#endif

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "../iolib/iolib.h"

LUALIB_API int (luaL_loadfilex) (lua_State *L, const char *filename,
                                               const char *mode);

#define luaL_loadfile(L,f)	luaL_loadfilex(L,f,NULL)
#define BUFSIZ	1024
 
#ifdef _MODULE
MODULE(MODULE_CLASS_MISC, luabase, "lua");

typedef struct LoadF {
  int n;  /* number of pre-read characters */
  int fd;
  file_t *f;  /* file being read */
  char buff[BUFSIZ];  /* area for reading file */
} LoadF;


static const char *getF (lua_State *L, void *ud, size_t *size) {
  LoadF *lf = (LoadF *)ud;
  size_t sz = 0;
  (void)L;  /* not used */
  if (lf->n > 0) {  /* are there pre-read characters to be read? */
    *size = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  else {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
	kread(lf->fd, lf->buff, sizeof(lf->buff), &sz); /* read block */ 
    if (sz == 0) 
		return NULL;
	*size = sz;
  }
  return lf->buff;
}


static int errfile (lua_State *L, const char *what, int fnameindex) {
  //XXX const char *serr = strerror(errno);
  const char *filename = lua_tostring(L, fnameindex) + 1;
  lua_pushfstring(L, "cannot %s %s: %s", what, filename);
  lua_remove(L, fnameindex);
  return LUA_ERRFILE;
}


static int skipBOM (LoadF *lf) {
  const char *p = "\xEF\xBB\xBF";  /* Utf8 BOM mark */
  int c;
  lf->n = 0;
  do {
    c = kgetc(lf->fd);
    if (c == -1 || c != *(const unsigned char *)p++) return c;
    lf->buff[lf->n++] = c;  /* to be read by the parser */
  } while (*p != '\0');
  lf->n = 0;  /* prefix matched; discard it */
  return kgetc(lf->fd);  /* return next character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
static int skipcomment (LoadF *lf, int *cp) {
  int c = *cp = skipBOM(lf);
  if (c == '#') {  /* first line is a comment (Unix exec. file)? */
    do {  /* skip first line */
      c = kgetc(lf->fd);
    } while (c != -1 && c != '\n') ;
    *cp = kgetc(lf->fd);  /* skip end-of-line, if present */
    return 1;  /* there was a comment */
  }
  else return 0;  /* no comment */
}


LUALIB_API int luaL_loadfilex (lua_State *L, const char *filename,
                                             const char *mode) {
  LoadF lf;
  int status, readstatus;
  int c;
  int fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */

  if (filename == NULL) {
    //XXX
	/*lua_pushliteral(L, "=stdin");
    lf.f = stdin*/;
  }
  else {
    lua_pushfstring(L, "@%s", filename);
    lf.f = kfopen(filename, "r", &lf.fd);
    if (lf.f == NULL) return errfile(L, "open", fnameindex);
  }
  if (skipcomment(&lf, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add line to correct line numbers */
  //XXX
  //if (c == LUA_SIGNATURE[0] && filename) {  /* binary file? */
  //  lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
  //  if (lf.f == NULL) return errfile(L, "reopen", fnameindex);
  //  skipcomment(&lf, &c);  /* re-read initial portion */
  //}
  if (c != -1)
    lf.buff[lf.n++] = c;  /* 'c' is the first character of the stream */
  status = lua_load(L, getF, &lf, lua_tostring(L, -1), mode);
  //XXX readstatus = ferror(lf.f);
  if (filename) kclose(lf.fd);  /* close file (even in case of errors) */
  //XXX if (readstatus) {
  //  lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
  //  return errfile(L, "read", fnameindex);
  //}
  lua_remove(L, fnameindex);
  return status;
} 

static int dofilecont (lua_State *L, int d1, lua_KContext d2) {
  (void)d1;  (void)d2;  /* only to match 'lua_Kfunction' prototype */
  return lua_gettop(L) - 1;
}

static int luaB_dofile (lua_State *L) {
  const char *fname = luaL_optstring(L, 1, NULL);
  lua_settop(L, 1);
  if (luaL_loadfile(L, fname) != LUA_OK)
    return lua_error(L);
  lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  return dofilecont(L, 0, 0);
} 

static int load_aux (lua_State *L, int status, int envidx) {
  if (status == LUA_OK) {
    if (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      if (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_pop(L, 1);  /* remove 'env' if not used by previous call */
    }
    return 1;
  }
  else {  /* error (message is on top of the stack) */
    lua_pushnil(L);
    lua_insert(L, -2);  /* put before error message */
    return 2;  /* return nil plus error message */
  }
}

static int luaB_loadfile (lua_State *L) {
  const char *fname = luaL_optstring(L, 1, NULL);
  const char *mode = luaL_optstring(L, 2, NULL);
  int env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  int status = luaL_loadfilex(L, fname, mode);
  return load_aux(L, status, env);
}

static int
luaopen_kbase(lua_State *L)
{
	const luaL_Reg base_lib[] = {
		{"dofile", luaB_dofile},
		{"loadfile", luaB_loadfile},
		{NULL, NULL}
	};

	luaL_newlib(L, base_lib);

	return 1;
}

static int
luabase_modcmd(modcmd_t cmd, void *opaque)
{
	int error;

	switch (cmd) {
	case MODULE_CMD_INIT:
		error = klua_mod_register("base", luaopen_kbase);
		break;
	case MODULE_CMD_FINI:
		error = klua_mod_unregister("base");
		break;
	default:
		error = ENOTTY;
	}
	return error;
}
#endif
