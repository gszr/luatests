/*
 * See copyright notice in LICENSE
 */

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

/* std descriptors reference /dev/null */
#define FD_STDIN	0
#define FD_STDOUT	1
#define FD_STDERR	2

#define IO_PREFIX	"_IO_"
#define IO_INPUT	(IO_PREFIX "input")
#define IO_OUTPUT	(IO_PREFIX "output")
#define IOPREF_LEN	(sizeof(IO_PREFIX)/sizeof(char) - 1)

#define tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))
#define isclosed(p) ((p)->closef == NULL)

#define l_checkmode(mode) \
	(*mode != '\0' && strchr("rwa", *(mode++)) != NULL &&	\
	(*mode != '+' || ++mode) &&  /* skip if char is '+' */	\
	(*mode != 'b' || ++mode) &&  /* skip if char is 'b' */	\
	(*mode == '\0'))
 

typedef luaL_Stream LStream;

#ifdef _MODULE
MODULE(MODULE_CLASS_MISC, luaio, "lua");

/* io.output */
static LStream*
tofile (lua_State *L) {
  LStream *p = tolstream(L);
  if (isclosed(p))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p);
  return p;
} 


/*
** function to close regular files
*/
static int 
io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  int res = kclose(p->fd);
  return luaL_fileresult(L, (res == 0), NULL);
}
 
/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** file is not left opened.
*/
static LStream*
newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdata(L, sizeof(LStream));
  if (p)
  p->closef = NULL;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  return p;
}
 
static LStream*
newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = NULL;
  p->closef = &io_fclose;
  return p;
}            

static LStream*
opencheck (lua_State *L, const char *fname, const char *mode) {
	LStream *p = newfile(L);
	int fd;
	//XXX handle modes
	p->f = kfopen(fname, mode, &fd);
	p->fd = fd;
  	if (p->f == NULL)
    	luaL_error(L, "cannot open file '%s'", fname);
	return p;
}

static int 
g_iofile (lua_State *L, const char *f, const char *mode) {
 	if (!lua_isnoneornil(L, 1)) {
		const char *filename = lua_tostring(L, 1);
    	if (filename)
      		opencheck(L, filename, mode);
    	else {
      		tofile(L);  /* check that it's a valid file handle */
      		lua_pushvalue(L, 1);
    	}
    	lua_setfield(L, LUA_REGISTRYINDEX, f);
  	}
  	/* return current value */
  	lua_getfield(L, LUA_REGISTRYINDEX, f);
  	return 1;
}

//XXX liolib
static int 
io_output (lua_State *L) {
	return g_iofile(L, IO_OUTPUT, "w");
}
/* */

/* io.close */
static int aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  volatile lua_CFunction cf = p->closef;
  p->closef = NULL;  /* mark stream as closed */
  return (*cf)(L);  /* close it */
}
 
static int io_close (lua_State *L) {
  if (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use standard output */
  tofile(L);  /* make sure argument is an open stream */
  return aux_close(L);
}
/* */


/* io.write starts here */

static LStream*
getiofile (lua_State *L, const char *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  if (isclosed(p))
    luaL_error(L, "standard %s file is closed", findex + IOPREF_LEN);
  return p;
}
 

static int g_write (lua_State *L, LStream *f, int arg) {
  int nargs = lua_gettop(L) - arg;
  int status = 1;
  for (; nargs--; arg++) {
    #if 0
	if (lua_type(L, arg) == LUA_TNUMBER) {
      /* optimization: could be done exactly as for strings */
      int len = lua_isinteger(L, arg)
                ? fprintf(f, LUA_INTEGER_FMT, lua_tointeger(L, arg))
                : fprintf(f, LUA_NUMBER_FMT, lua_tonumber(L, arg));
      status = status && (len > 0);
    }
    else {
	#endif
      size_t l;
	  size_t w;
      const char *s = luaL_checklstring(L, arg, &l);
      status = status && (kwrite(f->fd, (void*)s, l, &w) == 0) && (w == l);
    
  }
  if (status) return 1;  /* file handle already on stack top */
  else return luaL_fileresult(L, status, NULL);
}

static int io_write (lua_State *L) {
  return g_write(L, getiofile(L, IO_OUTPUT), 1);
}

static int f_write (lua_State *L) {
	LStream *f = tofile(L);
	lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
	return g_write(L, f, 2);
}

/* io.read starts here */
static int read_line (lua_State *L, LStream *f, int chop) {
  luaL_Buffer b;
  int c = '\0';
  luaL_buffinit(L, &b);
  while (c != -1 && c != '\n') {  /* repeat until end of line */
    char *buff = luaL_prepbuffer(&b);  /* pre-allocate buffer */
    int i = 0;
    //XXX l_lockfile(f);  /* no memory errors can happen inside the lock */
    while (i < LUAL_BUFFERSIZE && (c = kgetc(f->fd)) != -1 && c != '\n')
      buff[i++] = c;
    //XXX l_unlockfile(f);
    luaL_addsize(&b, i);
  }
  if (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, c);  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  return (c == '\n' || lua_rawlen(L, -1) > 0);
}

static void read_all (lua_State *L, LStream *f) {
  size_t nr;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  do {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    char *p = luaL_prepbuffsize(&b, LUAL_BUFFERSIZE);
    kread(f->fd, (void*) p, LUAL_BUFFERSIZE, &nr);
    luaL_addsize(&b, nr);
  } while (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}
              
static int read_chars (lua_State *L, LStream *f, size_t n) {
  size_t nr;  /* number of chars actually read */
  char *p;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  kread(f->fd, (void*) p, n, &nr);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  return (nr > 0);  /* true iff read something */
}
 

static int g_read (lua_State *L, LStream *f, int first) {
  int nargs = lua_gettop(L) - 1;
  int success;
  int n;
  //clearerr(f);
  if (nargs == 0) {  /* no arguments? */
    success = read_line(L, f, 1);
    n = first+1;  /* to return 1 result */
  }
  else {  /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    for (n = first; nargs-- && success; n++) {
      if (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)luaL_checkinteger(L, n);
        //XXX 
		success = (l == 0) ? 1 : read_chars(L, f, l);
      }
      else {
        const char *p = luaL_checkstring(L, n);
        if (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        switch (*p) {
          //XXX
		  #if 0
		  case 'n':  /* number */
            success = read_number(L, f);
            break;
		  #endif
          case 'l':  /* line */
            success = read_line(L, f, 1);
            break;
          case 'L':  /* line with end-of-line */
            success = read_line(L, f, 0);
            break;
          case 'a':  /* file */
            read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            break;
          default:
            return luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  //XXX
  /*(if (ferror(f))
    return luaL_fileresult(L, 0, NULL);*/
  if (!success) {
    lua_pop(L, 1);  /* remove last result */
    lua_pushnil(L);  /* push nil instead */
  }
  return n - first;
}


static int io_read (lua_State *L) {
  return g_read(L, getiofile(L, IO_INPUT), 1);
} 
/* */

/* io.input starts here */
static int io_input (lua_State *L) {
  return g_iofile(L, IO_INPUT, "r");
}
/* */

static int f_read (lua_State *L) {
	return g_read(L, tofile(L), 2);
}

static int io_open (lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  const char *mode = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  const char *md = mode;  /* to traverse/check mode */
  int fd;
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  p->f = kfopen(filename, mode, &fd);
  p->fd = fd;
  return (p->f == NULL) ? luaL_fileresult(L, 0, filename) : 1;
}
 

static void createmeta (lua_State *L, const luaL_Reg m[]) {
	luaL_newmetatable(L, LUA_FILEHANDLE);  /* create metatable for file handles */
	lua_pushvalue(L, -1);  /* push metatable */
	lua_setfield(L, -2, "__index");  /* metatable.__index = metatable */
	luaL_setfuncs(L, m, 0);  /* add file methods to new metatable */
	lua_pop(L, 1);  /* pop new metatable */
}


/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
static int io_noclose (lua_State *L) {
  LStream *p = tolstream(L);
  p->closef = &io_noclose;  /* keep file opened */
  lua_pushnil(L);
  lua_pushliteral(L, "cannot close standard file");
  return 2;
}

static int f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  if (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  else
    lua_pushfstring(L, "file (%p)", p->f);
  return 1;
}

static int f_gc (lua_State *L) {
  LStream *p = tolstream(L);
  if (!isclosed(p) && p->f != NULL)
    aux_close(L);  /* ignore closed and incompletely open files */
  return 0;
} 
 

static void 
createstdfile (lua_State *L, file_t *f, const char *k,
                           const char *fname) {
  LStream *p = newprefile(L);
  p->f = f;
  p->closef = &io_noclose;
  if (k != NULL) {
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
  }
  lua_setfield(L, -2, fname);  /* add file to module */
}
 

static int
luaopen_kio(lua_State *L)
{
	static const luaL_Reg flib[] = {
		{"close", io_close},
  		{"write", f_write},
		{"read", f_read},
		{"__gc", f_gc},
		{"__tostring", f_tostring},
		{NULL,  NULL}
	};
 
	createmeta(L, flib);

	const luaL_Reg io_lib[] = {
		{"open", io_open},
		{"close",  io_close},
		{"input", io_input},
		{"output", io_output},
		{"read", io_read},
		{"write",  io_write},
		{NULL, NULL}
	};

	luaL_newlib(L, io_lib);

	//XXX in, our, err reference /dev/null
 	createstdfile(L, kfdopen(FD_STDIN), IO_INPUT, "stdin");
  	createstdfile(L, kfdopen(FD_STDOUT), IO_OUTPUT, "stdout");
  	createstdfile(L, kfdopen(FD_STDERR), NULL, "stderr");

	return 1;
}

static int
luaio_modcmd(modcmd_t cmd, void *opaque)
{
	int error;

	switch (cmd) {
	case MODULE_CMD_INIT:
		error = klua_mod_register("io", luaopen_kio);
		break;
	case MODULE_CMD_FINI:
		error = klua_mod_unregister("io");
		break;
	default:
		error = ENOTTY;
	}
	return error;
}
#endif
