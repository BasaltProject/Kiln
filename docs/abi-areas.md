# ABI areas, the import table and the SDK bindings

The plugin ABI is the set of headers in `wire/wire-wasm/abi/`: the core (`basalt_abi.h`, `basalt_proxy_abi.h`) and one
header per area in `abi/areas/`. Everything below is derived from those headers; nothing lists an import by hand.

## How Kiln checks imports

`cmake/KilnAbi.cmake` calls Wire's parser (`wire/cmake/BasaltWasmAbi.cmake`) over every header at configure time and
writes `AbiTable.cpp` into the build directory. Each row is `{name, parameters, result, shared}`; `kImports` holds the
module `basalt` (`shared` marks what the proxy profile offers too) and `kProxyImports` the module `basalt_proxy`.
`WasmCheck.cpp` reads only that table, so a new `abi/areas/<area>.h` is picked up by the next configure (the glob
re-runs by itself) and `kiln check` accepts its imports for the right profile with no edit here.

## SDK bindings

`scripts/gen-sdk.py` turns each `abi/areas/<area>.h` into raw import declarations per language, and writes the small
index files that list the areas and the hand-written wrappers that exist. Run it after adding or changing an area header
or a wrapper, and commit the output; `python3 scripts/gen-sdk.py --check` (the ctest `kiln-sdk-generated`) fails when a
committed file is out of date. Never edit a generated file.

`<profile>` below is `server` or `proxy`. Public imports are listed in [Languages](languages.md).

| Language | Generated raw declarations | Wrapper implementation |
|---|---|---|
| C | ABI headers directly | `sdk/c/basalt/<profile>/<area>.h`, shared helpers in `basalt/internal` |
| C++ | C layer | `sdk/cpp/basalt/<profile>/<area>.hpp`, common templates in `sdk/cpp/shared` |
| Zig | `sdk/zig/basalt_<profile>/raw/<area>.zig` | `sdk/zig/basalt_<profile>/<area>.zig`, shared modules in `basalt_shared` |
| Rust | `sdk/rust/basalt_<profile>/raw/<area>.rs` | `sdk/rust/basalt_<profile>/<area>.rs`, shared modules in `common` |
| Go | `sdk/go/<profile>/internal/runtime/zz_<area>_raw.go` | `sdk/go/<profile>/<area>`, implementation templates in `sdk/go/shared` |
| Java | `sdk/java/<profile>/org/basaltmc/<profile>/internal/Raw<Area>.java` | `sdk/java/<profile>/org/basaltmc/<profile>/<area>`, common templates in `sdk/java/shared` |
| Kotlin | `sdk/kotlin/<profile>/internal/<Area>Raw.kt` | `sdk/kotlin/<profile>/<area>`, common templates in `sdk/kotlin/shared` |
| C# | `sdk/csharp/<profile>/Internal/Raw<Area>.cs` | `sdk/csharp/<profile>/<Area>`, common templates in `sdk/csharp/shared` |
| AssemblyScript | `sdk/assemblyscript/basalt-<profile>/raw/<area>.ts` | `sdk/assemblyscript/basalt-<profile>/<area>.ts`, shared modules in `basalt-internal` |

Shared template files end in `.in`; `gen-sdk.py` instantiates them under both product roots. Edit the template, then
regenerate. C has generated umbrella headers `basalt/server.h` and `basalt/proxy.h`; Rust has a generated area module
list, and Java's C glue includes `sdk/java/glue/generated/basalt_java_areas.h`.

Mapping: every pointer is an `i32` offset (typed pointer in Zig, Rust, Go, C#; `Address` in Java; `Int` in Kotlin;
`usize` in AssemblyScript), `int32_t`, `int64_t`, `float` and `double` map to the language's own types, and a parameter
name that is a keyword gets a trailing underscore. The generated Go, Java, Kotlin and C# names are `raw<PascalName>`.

The core imports (the hand-written internal `raw` blocks) are not generated; areas never redeclare them. Kotlin
and C# wrappers can use the generated declarations because they compile in the same module as the SDK. The wrapper
folders are optional: an area with no wrapper in a language simply has the raw declarations available.

Every language but C and C++ gets generated raw declarations; those two include the ABI header directly.
The generated files were checked to compile for every language with `kiln build` against a probe area (Go only needs
the package to compile, its raw functions are unexported and dropped when unused).

Kotlin keeps every `@WasmImport`, including unused declarations. The generator writes shared area bindings into both
product roots and product-specific bindings only into their own root. Kiln clears the extracted SDK before every build,
then extracts only the chosen product and shared internals; a previous build cannot leave declarations from another
profile behind. The `BASALT_IMPORT_SHARED`, `BASALT_IMPORT` and `BASALT_PROXY_IMPORT` macros remain the source of truth.

## Values in the SDK

Every SDK has a builder and a reader over the byte format of `abi/basalt_values.h` (the reference host codec is
`Value.hpp`). They are not areas: the index files include them always, and a wrapper uses them without importing
anything. All nine produce byte-identical documents and read the same bytes.

| Language | File | Writer | Reader | Call glue |
|---|---|---|---|---|
| C | `sdk/c/basalt/internal/values.h` | `bs_vw` over your buffer (`bs_vw_init`, `bs_vw_i64`, `bs_vw_list`, `bs_vw_key`, `bs_vw_end`, `bs_vw_finish`) | `bs_val` (`bs_value_open`, `bs_value_as_*`, `bs_value_find`, `bs_value_at`, `bs_value_iter` + `bs_iter_next`) | `bs_out_call`, `bs_out_value` (primary and spare buffer) |
| C++ | `sdk/cpp/shared/values.hpp.in` | `basalt::server::values::ValueWriter`, `ValueBuffer<N>` | `basalt::server::values::Value`, `ValueIter` | `call_out`, `call_value` (lambda) |
| Zig | `sdk/zig/basalt_shared/values.zig` (`basalt_server.values`) | `Writer.init(buf)` or `Writer.initAlloc(allocator)` | `Value.open`, optional getters, `items()` | `callOut`, `callOutAlloc`, `callValue` |
| Rust | `sdk/rust/common/values.rs` (`basalt_server::values`) | `Writer::new()` or `with_limit(n)` | `Value::open`, `Option` getters, `iter()`, `entries()` | `call_out`, `call_value` (closure) |
| Go | `sdk/go/shared/values.go.in` | `NewValueWriter()` or `NewValueWriterLimit(n)` | `OpenValue`, `(v, ok)` getters, `Iter()` | `CallOut`, `CallValue` (func) |
| Java | `sdk/java/server/org/basaltmc/server/values/Values.java` | `Values.Writer` | `Values.Value`, `Values.Iter` | `Values.callOut` (lambda) |
| Kotlin | `sdk/kotlin/shared/values/Values.kt.in` | `ValueWriter` | `Value`, `ValueIter` | `callOut` (lambda with the allocator receiver) |
| C# | `sdk/csharp/server/Values/Values.cs` | `ValueWriter` | `Value.TryOpen`, `Try*` getters, `Iter()` | `ValueCall.Out` (delegate) |
| AssemblyScript | `sdk/assemblyscript/basalt-internal/values.ts` | `ValueWriter` | `Value.open`, `to*` getters with fallbacks | `callOut(OutCall)` |

Shape shared by all of them:

- Writer: `null`, `bool`, `int` (i64), `float` (f64), `string`, `bytes`, `list`/`map` ... `end`, and `key` before each
  map value. The count of a container is inserted at `end`, so nothing needs to be known up front. Failure is sticky
  (`Invalid` for a misuse or a limit, `Overflow` for a full fixed buffer or a `limit`); `finish` returns the document or
  the error. Duplicate map keys are the caller's responsibility (the host rejects them).
- Reader: `open` validates the whole document once (tags, 1 MiB, depth 16, 65536 nodes, 256 byte keys, duplicate keys,
  trailing bytes) and answers absent or an error for anything malformed; it never traps. After that the getters work in
  place: scalar getters answer absent for another kind, `count`, `at(i)`, `find(key)` and iteration over a list or map.
- Call glue: an import that answers `(out, cap)` is wrapped in a function that calls it, grows the buffer to the
  needed length and calls once more, returning the bytes or the negative error (`-100` when the answer is over 1 MiB).
  The C family has no heap, so they take a second buffer instead of growing.
