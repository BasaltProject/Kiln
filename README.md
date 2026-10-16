# Kiln

Compiles the Lua of a Basalt plugin to bytecode (`.luac`), with the same Lua 5.5.1 the server runs.

```
kiln compile src/main.lua              writes src/main.luac
kiln compile src -o build/lua --strip  every .lua under src, same layout, no debug info
kiln check src                         syntax check only
```

The output is the same bytes every time for the same source. Without `--strip` the bytecode keeps line numbers
and the chunk name, so a runtime error still points at `file.lua:line`.

## Build

```
git submodule update --init
cmake --preset release && cmake --build build/release
```

## Targets

`--target server` (the default) and `--target proxy` pick the product the plugin is for. Both produce the same
Lua 5.5.1 bytecode today; the list lives in `src/Targets.hpp`, so a product that needs its own step adds a row there
and a hook in `src/Main.cpp` without touching the compiler.
