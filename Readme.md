
Applua
=======

This utility was meant to facilitate the deploying of applications based on a
"Standalone" lua distribution.  You can compile it upon the standard lua or
luajit, as well as any C implementation of the lua API.

A secondary goal was to substitute the standard lua command line tool with a
simpler one without a REPL. It neither parses in any way the command line
argument: it can be done in lua. With the help of some library with lua binding
(e.g. libuv) also a REPL can be implemented.

Any file without an explicit license reference is in the Public Domain. The
MIT-0 license can be alternatively applied. Please see the COPYING.txt file for
more informations.

Build and usage
----------------

There is no actual build system. You can compile it with gcc using:

```
gcc -std=c99 -I . -o applua.exe *.c lua_lib -lm -ldl
```

This assumes that you have copied the lua headers in the current directoy and
the binary library (static or shared) in the lua_lib file. For windows you can
add -D_WIN32_WINNT=0x0600 -mconsole (or -mwindows if you do not want a console
to appear at start of the application).

// TODO : document gcc linker ORIGIN

The result app, `applua.exe`, will simply load the `lua.init` file in the same
directory and run it as lua source/bytecode. If that file can not be found,
applua will try a file with same name of the binary but the `lua` extension.

The script will have access to the common lua globals, plus an `whereami`
variable. This contains the absolute path to the executable file.

Note that you can find similar tools around the web, e.g.
[l-bia](http://l-bia.sourceforge.net) (but I never tryed it).

