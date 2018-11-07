
#ifndef SHFUNC
  #define SHFUNC
#endif

typedef struct lua_State lua_State;
SHFUNC lua_State* luamain_setup(lua_State *L, int argc, char **argv);
SHFUNC int luamain_exec(lua_State *L, char* script, char* src, int lin);

