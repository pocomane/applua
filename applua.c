
#include <string.h>
#include <errno.h>

#include "whereami.h"
#include "luamain.h"
#include "lauxlib.h"

static int back_search(const char * str, const char * chs, int pos){
  int found = 0;
  int i;
  if (pos < 0) pos = strlen(str);
  for (i = pos; !found && i > 0; i--)
    for (int j = 0; !found && chs[j] != '\0'; j += 1)
      if (str[i] == chs[j])
        found = 1;
  if (found) i += 1;
  return i;
}

int main(int argc, char** argv) {

  lua_State * L = luamain_setup(0, argc, argv);

  char * exe_path = argv[0];
  int length;
  length = wai_getExecutablePath(NULL, 0, NULL);
  char buf[length+1];
  buf[0] = '\0';
  if (length > 0) {
    exe_path = buf;
    int dirpathlen;
    wai_getExecutablePath(exe_path, length, &dirpathlen);
    exe_path[length] = '\0';
    lua_pushstring(L, exe_path);
    lua_setglobal(L, "whereami");
  }
  length = strlen(exe_path);

  char init_path[length+16];
  strncpy(init_path, exe_path, length+16);
  int i = 1 + back_search(init_path, "/\\", length-1);
  init_path[i] = '\0';
  strncpy(init_path+i, "lua.init", 16);

  char script[strlen(init_path)+32];
  snprintf(script, sizeof(script), "(loadfile[[%s]])()\n", init_path);

  return luamain_exec(L, script, __FILE__, __LINE__);
}

