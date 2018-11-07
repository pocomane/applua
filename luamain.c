
#include "luamain.h"

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "lua.h"
#include "lauxlib.h"

#define FAIL_INIT -125
#define FAIL_ALLOC -126
#define FAIL_EXECUTION -127
#define ALL_IS_RIGHT 0

#ifdef LUA_OK
 #define is_lua_ok(status_code) (LUA_OK == status_code)
#else
 #define is_lua_ok(status_code) (!status_code)
#endif

#ifdef LUA_OK
  #define lua_is_bad() (!LUA_OK)
#else
  #define lua_is_bad() (is_lua_ok(0) ? 1 : 0)
#endif

static int msghandler (lua_State *L) {

  // is error object not a string?
  const char *msg = lua_tostring(L, 1);
  if (msg == NULL) {

    // call a tostring metamethod if any
    if (luaL_callmeta(L, 1, "__tostring") && lua_type(L, -1) == LUA_TSTRING){
      msg = lua_tostring(L, -1);
      lua_remove(L, -1);

    // else push a standard error message
    } else {
      msg = lua_pushfstring(L, "(error object is a %s value)", luaL_typename(L, 1));}
  }

  // append a traceback
  luaL_traceback(L, L, msg, 1);

  return 1;
}

// Signal hook: stop the interpreter. Just like standard lua interpreter.
static void clear_and_stop(lua_State *L, lua_Debug *ar) {
  (void)ar;  // unused arg.
  lua_sethook(L, NULL, 0, 0);
  luaL_error(L, "interrupted!");
}
static lua_State *globalL = NULL;
static void sigint_handler (int i) {
  signal(i, SIG_DFL); // if another SIGINT happens, terminate process
  lua_State *L = globalL;
  globalL = NULL;
  lua_sethook(L, clear_and_stop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1); // Run 'clear_and_stop' before any other lua code
}

static void report_error(lua_State *L, const char * title){
  if (!title || title[0] == '\0')
    title = "An error occurred somewhere.\n";
  char * data = "No other information are avaiable\n";
  if (lua_isstring(L, -1)) data = (char *) lua_tostring(L, -1);
  lua_getglobal(L, "print");
  lua_pushstring(L, title);
  lua_pushstring(L, data);
  lua_pcall(L, 2, 1, 0);
}

SHFUNC int luamain_exec(lua_State *L, char* script, char* src, int lin) {
  int base = 0;
  int status;
  int size = strlen(script);

  // os signal handler
  void * old_handler = signal(SIGINT, NULL);
  globalL = L;  // to be available to 'sigint_handler'
  signal(SIGINT, sigint_handler);  // set C-signal handler

  // Prepare the stack with the error handler
  lua_pushcfunction(L, msghandler);
  base = lua_gettop(L);

  // Load the script in the stack
  if (size < 0) size = strlen(script);
  if (src && lin) {
    char info[strlen(src)+32];
    snprintf(info, sizeof(info), "emedded:%s:%d\n", src, lin);
    status = luaL_loadbuffer(L, script, size, info);
  } else {
    status = luaL_loadbuffer(L, script, size, "embedded");
  }
  if (!is_lua_ok(status)) {
    report_error(L, "An error occurred during the script load.");
    status = FAIL_EXECUTION;
    goto luamain_end;
  }

  // Run the script with the signal handler
  status = lua_is_bad();
  status = lua_pcall(L, 0, LUA_MULTRET, base);
  if (is_lua_ok(status)) {
    status = ALL_IS_RIGHT;
    goto luamain_end;
  }

  // Report error
  report_error(L, "An error accurred during the script execution.");
  if (lua_isnumber(L, -1)) status = lua_tonumber(L, -1);
  else status = FAIL_EXECUTION;

luamain_end:

  // clear C-signal handler
  signal(SIGINT, old_handler);
  globalL = NULL;

  if (base>0) lua_remove(L, base);  // remove lua message handler
  return status;
}

SHFUNC lua_State* luamain_setup(lua_State *L, int argc, char **argv) {

  // create state as needed
  if (L == NULL) {
    L = luaL_newstate();
    if (L == NULL) return NULL;
  }

  luaL_openlibs(L);  // open standard libraries

  // Create a table to store the command line arguments
  lua_createtable(L, argc-1, 1);

  // Arg 0 : command-line-like path to the executable:
  // it may be a link and/or be relative to the current directory
  lua_pushstring(L, argv[0]);
  lua_rawseti(L, -2, 0);

  // Args N... : command line arguments
  for (int i = 1; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i);
  }

  // Save the table in the global namespace
  lua_setglobal(L, "arg");

  return L;
}

SHFUNC int luamain_start(lua_State *L, char* script, int size, int argc, char **argv) {
  int create_lua = 0;
  if (L == NULL)
    create_lua = 1;
  L = luamain_setup(L, argc, argv);
  if (L == NULL) return FAIL_ALLOC;
  int status = luamain_exec(L, script, 0, 0);
  if (create_lua) lua_close(L);
  return status;
}

