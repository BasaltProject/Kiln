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
  <img alt="C++" src="https://img.shields.io/badge/C%2B%2B-23-00599C?logo=cplusplus&logoColor=white">
  <img alt="Lua" src="https://img.shields.io/badge/Lua-5.5.1-000080?logo=lua&logoColor=white">
  <img alt="Platforms" src="https://img.shields.io/badge/platforms-macOS_%7C_Linux_%7C_Windows-00599C">
  <img alt="Build" src="https://img.shields.io/badge/build-CMake-064F8C?logo=cmake&logoColor=white">
  <img alt="License" src="https://img.shields.io/badge/license-GPL--3.0-green">
</p>

## Overview

Kiln is part of the Basalt Project. It does two jobs for a plugin folder:

- **Verifies it.** It reads `plugin.toml`, every Lua file under `src/`, each `require` and every name the code takes
  from the Basalt API, and reports what is wrong before the plugin ever reaches a server.
- **Compiles it** to Lua bytecode (`.luac`) with the same Lua 5.5.1 the server runs, so the bytecode loads there as is.

The output is the same bytes every time for the same sources.

## Building

Requires CMake 3.28 or newer, Ninja and Clang.

```bash
git submodule update --init
cmake --preset release && cmake --build build/release
```

## Usage

```
kiln check <plugin|file.lua|dir>      verify, write nothing
kiln compile <plugin|file.lua|dir>    verify, then write the bytecode
```

| Option | Meaning |
|---|---|
| `-o <path>` | Where the bytecode goes. A plugin folder defaults to `build/luac`, keeping the layout of `src/` |
| `--strip` | Drop debug information. Without it an error still points at `file.lua:line` |
| `--target server\|proxy` | The product the plugin is for. Defaults to `server` |
| `--no-verify` | Compile a plugin folder without the checks below |

Given a folder with a `plugin.toml`, Kiln treats it as a plugin. Given a file or any other folder, it only checks
syntax and compiles.

## Building and signing

```
kiln keygen release                          writes release.key (secret) and release.pub
kiln build <plugin> --sign release.key       verify, compile and sign into build/compiled
```

`build` leaves a plugin folder with the manifest, the resources and bytecode only, plus a `plugin.sig` holding an
Ed25519 signature over the manifest and every `.luac`. The server refuses bytecode that carries no signature, a
signature that does not match, or a key missing from `trusted-keys` under `[plugins]` in `config/server.toml`; the
public key from `release.pub` goes there. `-o <dir>` picks the output and `--strip` drops debug information.

## What it checks

```
$ kiln check arena
src/main.lua:1: no module `missing` in src/
src/main.lua:2: unknown global `onn`
src/main.lua:3: `log.infoo` is not part of the API
src/main.lua:5: `os.execute` is not part of the API
4 problem(s)
```

- **Syntax** of every Lua file.
- **`plugin.toml`**: `id`, `name`, `version` and `main` are present, `id` is made of lowercase letters, digits, `_`
  and `-`, and the main file exists.
- **`require`** of a literal name resolves to a file under `src/`.
- **The API**: a global or a field that the target does not offer, such as a typo or a function the sandbox removed.
  A name the plugin defines itself is accepted.

The check follows names through the bytecode, so it does not guess at what a variable holds: only paths like
`log.info` or `players.online` are verified, and a table with no listed members (`config`, `store`) accepts any key.

## Targets and API manifests

The names a plugin may use live in one file per target, `api/<target>.api`:

```
# path kind
log table
log.info function
plugin.name string
```

`kind` is `table`, `function` or any other word for a plain value. A table lists its members as further lines; a
table with none is open. A file that starts a line with `@permissive` makes Kiln report no unknown names for that
target, which is how a target starts before its API is written down.

`api/server.api` is taken from the running server sandbox. `api/proxy.api` holds the Lua standard library and is
permissive until the proxy lists its own names.

### Adding a target

1. Add a row to `kTargets` in `src/Targets.hpp`.
2. Add `api/<target>.api`, and read it in `CMakeLists.txt` and `src/Manifests.cpp.in` next to the other two.
3. A product that needs a step of its own hooks it in `src/Main.cpp`; the compiler and the analyzer stay shared.

## License

GPL-3.0-only. See [LICENSE](LICENSE).
