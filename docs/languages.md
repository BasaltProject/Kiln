# Languages

`kiln build` writes `build/<main>` (default `build/plugin.wasm`) from the sources and then runs the same checks as
`kiln check`. The language comes from `lang` in `plugin.toml` or, when absent, from the files. `kiln new <id> --lang
<language>` writes a working plugin (event, command, task, config, translate and menu) in any supported language.

Each SDK is embedded in Kiln and written to `build/.kiln/sdk/<lang>/` on every build; nothing is installed in the
plugin folder. The functions are the same in all languages up to case (`config_i64`, `configI64`, `ConfigI64`).

| Language | Id | Entry | Compiler | Status |
|---|---|---|---|---|
| C++ | `cpp` | `src/**/*.cpp`, `void plugin_init()` | clang, else `zig c++` | yes |
| C | `c` | `src/**/*.c`, `void plugin_init(void)` | clang, else `zig cc` | yes |
| Zig | `zig` | `src/main.zig`, `pub fn init() void` | `zig build-exe` | yes |
| Rust | `rust` | `src/lib.rs`, `pub fn init()` | `rustc`, no cargo | yes |
| AssemblyScript | `assemblyscript` | `src/main.ts`, `export function init()` | `asc` on node | yes |
| Go | `go` | `main.go`, `core.Init(setup)` | TinyGo | yes |
| Kotlin | `kotlin` | `src/**/*.kt`, packaged `fun init()` | `kotlinc-wasm` | partial |
| C# | `csharp` | `src/**/*.cs`, packaged `Plugin.Init()` | NativeAOT-LLVM on .NET 10 | partial |
| Java | `java` | `src/**/*.java`, packaged `Main.init()` | TeaVM (C backend) and Zig | partial |

## Proxy plugins

`kiln new <id> --lang <language> --target proxy` writes a proxy plugin (join event, command, task and player calls) with
`target = "proxy"` in `plugin.toml`; `build` and `check` then validate against the proxy profile and `kiln pack` writes
`<id>-<version>.bppkg`. Each SDK has separate Server and Proxy roots, with one package, module, namespace or header per function area.
A build extracts only the selected product and its shared internals. The WASM module and import names remain unchanged.

| Language | Server import | Proxy import |
|---|---|---|
| C | `<basalt/server/events.h>` | `<basalt/proxy/events.h>` |
| C++ | `<basalt/server/events.hpp>`, `basalt::server::events` | `<basalt/proxy/events.hpp>`, `basalt::proxy::events` |
| Zig | `@import("basalt_server").events` | `@import("basalt_proxy").events` |
| Rust | `basalt_server::events` | `basalt_proxy::events` |
| AssemblyScript | `basalt-server/events` | `basalt-proxy/events` |
| Go | `basaltmc.org/sdk/server/events` | `basaltmc.org/sdk/proxy/events` |
| Kotlin | `org.basaltmc.server.events` | `org.basaltmc.proxy.events` |
| Java | `org.basaltmc.server.events.Events` | `org.basaltmc.proxy.events.Events` |
| C# | `Basalt.Server.Events` | `Basalt.Proxy.Events` |

Common areas are `core`, `values`, `log`, `config`, `text`, `players`, `data`, `events`, `commands` and `tasks`.
Server adds `menus`, `worlds`, `entities`, `items`, `hud`, `packets` and `server`. Proxy adds `hud`, `interaction`,
`admin` and `network`; Zig keeps basic backend calls in `servers`.

## Notes per language

- **C and C++**: freestanding wasm32, no libc or exceptions. The SDK brings `memcpy` and friends. `-Wno-comment` hides a
  warning from the ABI header.
- **Zig**: Kiln generates a root file that exports the entry points and calls `init` of `src/main.zig`.
- **Rust**: `std` is available but the allocator is the default `dlmalloc`; the SDK is built as an rlib and
  the plugin as a `cdylib` with `panic=abort` and LTO. Kiln builds the SDK as the `basalt_server` or `basalt_proxy` crate without Cargo.
- **AssemblyScript**: the default `abort` import of the runtime is replaced by a trap so the module only imports from
  `basalt`. Handlers cannot be closures (an AssemblyScript limit).
- **Go**: TinyGo with `-target wasm-unknown -scheduler=none`; no goroutines. The module exports the reactor
  `_initialize`, which the host runs before `basalt_init`. The SDK module `basaltmc.org/sdk` is wired in
  with a generated `go.work` (outside the project), so the plugin's own `go.mod` stays plain. An editor needs the same
  `go.work` to resolve the import.
- **Kotlin**: Kotlin/Wasm for WASI, linked in two steps (klib, then module). The result is a WasmGC module of about
  650 KB because it carries the Kotlin runtime, so it needs a host with the GC proposal enabled (wasmtime `-W gc`
  or the default of recent versions). `kotlin.wasm.unsafe` is used for linear memory. This is why it is "partial".
  The module also exports a `_start` that the host never calls; the ABI does not need it.
- **Java**: TeaVM 0.17 compiles the classes to C (`-t C`), and Zig compiles that C for `wasm32-wasi` as a reactor with
  `--export-memory`, so the module exports a plain linear `memory` and the ABI holds unchanged. TeaVM's WasmGC backend is
  not used: it imports `env.memory` and has no linear memory for the ABI's buffers. The Eclipse compiler (ecj) compiles
  the sources, and a JRE 17 or newer is used from the machine or downloaded. The module is 140 to 190 KB. It is "partial"
  because the output uses the exception handling proposal (setjmp lowered by clang), which Wasmtime runs and WAMR does
  not load, so there is no Java on FreeBSD. Only the JDK subset TeaVM implements is available.
- **C#**: NativeAOT-LLVM (dotnet/runtimelab), not the .NET `wasi-wasm` workload: that one builds a Mono command with `_start`
  and, since .NET 11, a WASI component, neither of which exports plain functions or imports from a custom module.
  NativeAOT-LLVM compiles to a core module: `[UnmanagedCallersOnly(EntryPoint = ...)]` becomes an export and
  `[WasmImportLinkage, DllImport("basalt")]` an import. Kiln links with `wasm32-unknown-wasip1` instead of the default
  component target and defines the seven WASI functions the host lacks (`wasi_stubs.c` in the SDK). The module is about
  950 KB and carries the .NET runtime and its GC. It is "partial" because the compiler is experimental: its packages come
  from the `dotnet-experimental` feed (last published for .NET 10 preview 2, Oct 2025), it needs the .NET 10 SDK and the
  WASI SDK, and only linux-x64 is pinned. The plugin sources go through one `plugin.csproj` Kiln writes; invariant
  globalization is on and exceptions are emulated, so keep them out of callbacks.
