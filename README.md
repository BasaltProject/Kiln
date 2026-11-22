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
  <strong>Builds, checks and packs WebAssembly plugins for Basalt. Native, no JVM.</strong>
</p>

<p align="center">
  <img alt="C++" src="https://img.shields.io/badge/language-C%2B%2B-00599C?logo=cplusplus&logoColor=white">
  <img alt="WebAssembly" src="https://img.shields.io/badge/WebAssembly-654FF0?logo=webassembly&logoColor=white">
  <img alt="Platforms" src="https://img.shields.io/badge/platforms-macOS_%7C_Linux_%7C_Windows-00599C">
  <img alt="License" src="https://img.shields.io/badge/license-GPL--3.0-green">
</p>

## About

Kiln is the command line for writing plugins in C, C++, Zig, Rust, AssemblyScript, Go (TinyGo), Kotlin or C#. It
downloads the compiler a language needs the first time, checks the result against the plugin ABI without running it
and packs `.bspkg` (`.bppkg` for the proxy). See `docs/` for the languages and the toolchain cache.

```
kiln new <id> [name] --lang <language>   create a new plugin (--target proxy for the proxy)
kiln build [plugin]                      compile to build/plugin.wasm
kiln check [plugin|module.wasm]          verify plugin.toml and the module
kiln pack [plugin]                       verify, then pack a .bspkg or .bppkg
kiln toolchain list|install|path|clean   manage the downloaded compilers
```

```
$ kiln check arena
build/plugin.wasm: import basalt.player_sendd is not part of ABI 1
1 problem(s)
```

Use `--target server|proxy` (or `target = "proxy"` in `plugin.toml`; default `server`) to pick the product for
`new`, `build`, `check` and `pack`, `--offline` to forbid downloads and
`--cache-dir` or `KILN_HOME` to move the cache.

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
