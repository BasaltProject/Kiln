# kiln check

`kiln check [plugin|module.wasm]` validates without running anything. For a folder it reads `plugin.toml` first:

- `id`, `version`, `api = 1` and `abi = 1` are present and valid; `lang`, when set, is a known language;
- `main` ends in `.wasm`;
- every key of `[capabilities]` is a known name (`storage`, `players_data`, `commands`, `files`, `network`, `packets`,
  `worldgen`, `datapacks`; see ABI.md), the list being the one in Wire's manifest parser.

Then, if the module is built (next to the manifest or under `build/`), it parses the binary and checks:

- the target is `server` unless `plugin.toml` says `target = "proxy"` or `--target proxy` is given; a proxy plugin may
  import from `basalt_proxy` and only the shared `basalt` imports of the proxy profile (no menus, placeholders,
  particles, sounds or `store_top`), and a server plugin may not import from `basalt_proxy`;
- every import comes from module `basalt` or from the `wasi_snapshot_preview1` subset (`fd_write`, `clock_time_get`,
  `random_get`, `environ_get`, `environ_sizes_get`, `args_get`, `args_sizes_get`, `proc_exit`);
- every `basalt` import exists in the ABI header, is a function and has exactly its signature;
- an `env` import is rejected with a hint, since it means the toolchain linked against a host that does not exist;
- the module exports `memory` and does not import it, and exports `basalt_abi_version() -> i32`,
  `basalt_init()` and `basalt_callback(i32, i32)`; an optional `_initialize()` (the WASI reactor init, which the host
  calls before `basalt_init`) must have type `() -> ()`;
- limits: 16 MiB of module, 4096 initial memory pages, 2048 imports.

The tables of imports are generated at build time from `wire/wire-wasm/abi/basalt_abi.h` and `basalt_proxy_abi.h`, so
they cannot drift from the headers. The list of `basalt` imports shared with the proxy is the one in `ABI-proxy.md`. The parser understands WasmGC type sections, which Kotlin needs.
