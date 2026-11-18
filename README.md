<p align="center">
  <a href="https://raw.githubusercontent.com/BasaltProject/.github/main/assets/basalt-logo-white.svg#gh-dark-mode-only">
    <img src="https://raw.githubusercontent.com/BasaltProject/.github/main/assets/basalt-logo-white.svg" alt="Basalt" width="360">
  </a>
  <a href="https://raw.githubusercontent.com/BasaltProject/.github/main/assets/basalt-logo-black.svg#gh-light-mode-only">
    <img src="https://raw.githubusercontent.com/BasaltProject/.github/main/assets/basalt-logo-black.svg" alt="Basalt" width="360">
  </a>
</p>

<h2 align="center">Basalt Kiln</h2>

<p align="center">
  <strong>Compiles and verifies the Lua of a Basalt plugin. Native, no JVM.</strong>
</p>

<p align="center">
  <img alt="C++" src="https://img.shields.io/badge/language-C%2B%2B-00599C?logo=cplusplus&logoColor=white">
  <img alt="Lua" src="https://img.shields.io/badge/Lua-5.5.1-000080?logo=lua&logoColor=white">
  <img alt="Platforms" src="https://img.shields.io/badge/platforms-macOS_%7C_Linux_%7C_Windows-00599C">
  <img alt="License" src="https://img.shields.io/badge/license-GPL--3.0-green">
</p>

## About

Kiln is the command line for writing plugins. For a plugin folder it verifies `plugin.toml`, every Lua file, each
`require` and every name taken from the Basalt API, and reports what is wrong before the plugin reaches a server. It
compiles to Lua bytecode with the same Lua 5.5.1 the server runs, scaffolds new plugins, packs `.bspkg` (`.bppkg` for
the proxy), runs plugin tests and signs builds.

```
kiln check <plugin|file.lua|dir>      verify, write nothing
kiln compile <plugin|file.lua|dir>    verify, then write the bytecode
kiln new <id> [name]                  create a new plugin
kiln pack [plugin]                    verify, then pack a .bspkg
kiln test [plugin]                    verify, then run its tests
```

```
$ kiln check arena
src/main.lua:1: no module `missing` in src/
src/main.lua:3: `log.infoo` is not part of the API
2 problem(s)
```

Use `--target server|proxy` to pick the product (default `server`) and `kiln --help` for the rest.

## Download

Each release on the [releases page](https://github.com/BasaltProject/Kiln/releases) carries the executable and a
`SHA256SUMS` file. Put it on your `PATH` as `kiln`.

## Building

```bash
git submodule update --init
cmake --preset release && cmake --build build/release
```

Needs CMake 3.28+, Ninja and Clang.

## License

[GPL-3.0-only](LICENSE).
