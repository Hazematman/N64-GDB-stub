# N64 GDB Stub
This is a gdb stub for the n64 that works with a modified version of cen64. In order to properly use this stub you need
to use the fork of cen64 [here](https://github.com/Hazematman/cen64). This stub currently depends on libdragon.

You'll need to add the gdbstub.c and gdbstub_sys.c files to your project. If you execute the makefile contained in this
repo, it will build an example program using this stub.

When you start the cen64 emulator it will wait for a connection from gdbserver over unix domain sockets. The emulator
will create a unix domain socket called `debug_socket` in the folder it is executed from. When running gdb (built for
 mips64-elf) you can then execute the gdb command `target remote debug_socket` to connect.
 
Currently the gdb communication layer uses unix domain sockets. I would like to convert this to something more portable,
so that debugging will work on windows, linux, and mac.
 
## How to use the example project
This section assumes that you have already built the modified cen64 and a version of gdb for mips64-elf. In order 
to build this rom you first need to set the environment variable `N64_INST` to the path where you n64 toolchain
is installed. Once that is done you can run the command `make` to build the example rom which will be called `out.z64`.
To debug this rom you need to open cen64 with this rom. The emulator window will not open until the debugger is
connected. To connect the debug run `mips64-elf-gdb out.elf` (note that we use the elf file here and not the n64 rom,
this is required to get the right debug symbols). Once gdb has opened you will need to type in `target remote debug_socket`
(this step assumed that you have ran gdb from the same folder cen64 was run from). The debugger will then connect
and you will be able to use gdb commands to set breakpoints and inspect the target.

## How to use this stub
To use this stub you simply need to include the source files (gdbstub.c & gdbstub_sys.c) into your project. After they
are included you need to add a call to `dbg_main()` in your program near initilization to connect the debugger.

## TODO
- Implement stepping support. Currently only continue is supported. Cannot step lines of source code on the target
- Use generic socket interface so library will work on more operating systems
