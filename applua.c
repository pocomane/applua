
#include <string.h>
#include <errno.h>

#include "whereami.h"
#include "luamain.h"
#include "lauxlib.h"

int back_search(const char * str, const char * chs, int pos){
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

  lua_State * L = luaL_newstate();

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

  char * curr_scr = init_path;

  FILE *in = fopen(init_path, "rb");
  if (!in) {

    char spec_path[length+16];
    curr_scr = spec_path;
    strncpy(spec_path, exe_path, length+16);
    i = back_search(spec_path, ".", length-1);
    if (!(i > 0)) i = length;
    strncpy(spec_path+i, ".lua", 16);

    in = fopen(spec_path, "rb");
    if (!in) {

      fprintf(stderr, "Can not open neither %s or %s\n", init_path, spec_path);
      return -1;
    }
  }
  fseek(in, 0, SEEK_END);
  long size = ftell(in);
  if (fseek(in, 0, SEEK_SET)) {
    fprintf(stderr, "Can not access %s (%d): %s\n", curr_scr, errno, strerror(errno));
    return -1;
  }

  char script[size+1];
  if (1 != fread(script, size, 1, in)) {
    fprintf(stderr, "Can not read %s (%d): %s\n", curr_scr, errno, strerror(errno));
    return -1;
  }
  script[size] = 0;

  fclose(in);

  return luamain_start(L, script, size, argc, argv);
}

