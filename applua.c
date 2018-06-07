
#include <string.h>
#include <errno.h>

#include "whereami.h"
#include "luamain.h"
#include "lauxlib.h"

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
  int i;
  for (i = length; i > 0; i--){
    if (init_path[i] != '/' && init_path[i] != '\\') {
      init_path[i] = '\0';
    } else {
      break;
    }
  }
  strncpy(init_path+i+1, "init", 16);

  FILE *in = fopen(init_path, "rb");
  if (!in) {
    fprintf(stderr, "Can not open %s (%d): %s\n", init_path, errno, strerror(errno));
    return -1;
  }
  fseek(in, 0, SEEK_END);
  long size = ftell(in);
  if (fseek(in, 0, SEEK_SET)) {
    fprintf(stderr, "Can not access %s (%d): %s\n", init_path, errno, strerror(errno));
    return -1;
  }

  char script[size+1];
  if (1 != fread(script, size, 1, in)) {
    fprintf(stderr, "Can not read %s (%d): %s\n", init_path, errno, strerror(errno));
    return -1;
  }
  script[size] = 0;

  fclose(in);

  return luamain_start(L, script, size, argc, argv);
}

