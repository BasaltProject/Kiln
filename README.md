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

Kiln is part of the Basalt Project, the command line for writing plugins. For a plugin folder it:

- **Verifies it.** It reads `plugin.toml`, every Lua file under `src/`, each `require` and every name the code takes
  from the Basalt API, and reports what is wrong before the plugin ever reaches a server.
- **Compiles it** to Lua bytecode (`.luac`) with the same Lua 5.5.1 the server runs, so the bytecode loads there as is.

It also scaffolds a new plugin, packs it into a `.bspkg` and runs its tests. The output is the same bytes every time
for the same sources.

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
kiln new <id> [name]                  create <id>/ with plugin.toml, src/main.lua and resources/config.toml
kiln pack [plugin]                    verify, then pack build/<id>-<version>.bspkg
kiln test [plugin]                    verify, then run plugin/tests/*.lua
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

## Packing

`kiln pack` refuses a plugin with problems, then writes the manifest, the sources and the resources into
`build/<id>-<version>.bspkg`, the same bytes every time for the same files. Two resources with the same file name in
different folders are refused. With `--compiled` it packs the output of `kiln build` instead, so `--sign <key>` and
`--strip` apply and the package carries bytecode and `plugin.sig`.

## Testing

Every `tests/*.lua` runs in the plugin sandbox against a mock server, and tests are not packed:

```lua
require("main")                       -- loads src/main.lua, which registers its handlers

test("a player is welcomed", function()
    local alice = mock.player("Alice")
    mock.emit("player.join", { player = alice })
    assert(alice.sent[1].args.name == "Alice")
end)
```

- `require("name")` finds `src/name.lua`, as it does inside the server.
- `on`, `watch` and `cmd` record what the plugin registers. `every` and `later` schedule on a mock clock that
  `mock.tick(n)` advances.
- `worlds.by_name(name)` gives a small world that keeps `set_block`/`block` and the entities `spawn` makes.
- `mock.player(name, fields)` makes a stand-in player whose `send(text, args)` collects `sent`.
- `mock.emit(event, data)` and `mock.command(name, source, ...)` call what the plugin registered.
- A failed `assert` fails that test, and `kiln test` exits with 1 if any test fails.

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

`api/server.api` is taken from the running server sandbox, including the tables a plugin only gets with a capability
(`packet`, `channel`, `run`). The server keeps a copy of it for its own test, so a change here is made there too.
`api/proxy.api` holds the Lua standard library and the names the proxy offers. Neither shipped target uses
`@permissive` today.

### Adding a target

1. Add a row to `kTargets` in `src/Targets.hpp`.
2. Add `api/<target>.api`, and read it in `CMakeLists.txt` and `src/Manifests.cpp.in` next to the other two.
3. A product that needs a step of its own hooks it in `src/Main.cpp`; the compiler and the analyzer stay shared.

## License

GPL-3.0-only. See [LICENSE](LICENSE).
