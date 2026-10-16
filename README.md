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
