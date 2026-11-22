# Toolchains

Kiln builds a plugin with the compiler of its language and never asks you to install one. For each language it looks
for a suitable compiler already on the machine, and otherwise downloads a pinned one into a per-user cache.

## Resolution order

1. A compiler on `PATH` that can target wasm32 (checked by running it, not by its name). `clang` is accepted for C and
   C++ only when it can link wasm with `lld`; `rustc` only when the `wasm32-unknown-unknown` standard library is
   installed; `zig` from 0.14; `node` from 18; `java` from 17; `tinygo` together with `go`.
2. The pinned toolchain from the cache, downloaded on first use.

Never written outside the cache, never needs root. `kiln toolchain list` shows what is installed and what a download
costs.

## Cache

| Platform | Default location |
|---|---|
| Linux | `$XDG_CACHE_HOME/basalt/kiln`, else `~/.cache/basalt/kiln` |
| macOS | `~/Library/Caches/basalt/kiln` |
| Windows | `%LOCALAPPDATA%\basalt\kiln\cache` |

`KILN_HOME` or `--cache-dir <dir>` override it (the flag wins).

```
<root>/toolchains/<id>-<version>/   unpacked toolchain, with a .kiln-installed marker once complete
<root>/downloads/<sha256>[.part]    archives in flight; a .part file resumes with an HTTP Range request
<root>/locks/<id>.lock              held while one toolchain installs, so two builds never race
<root>/work/                        compiler caches (Zig, Go) kept out of the user's own
```

## Downloads

- Every archive has its SHA-256 pinned in `src/ToolchainPins.cpp`, which `scripts/update-pins.py` regenerates from the
  publishers' manifests. A mismatch deletes the file and fails the build.
- HTTPS is done in process (mbedTLS, certificates verified against `SSL_CERT_FILE`, the system bundle or the Windows
  root store), with up to 5 https-only redirects, resume and retries. No `curl`, `tar` or `unzip` is run.
- Extraction is in process too: tar.gz (zlib), tar.xz (a built-in xz and LZMA2 decoder) and zip. Absolute paths, `..`,
  backslashes, drive prefixes and symlinks that leave the destination are rejected.
- The toolchain is unpacked into a staging folder and renamed into place, so an interrupted install never looks
  installed.

`--offline` fails with the exact `kiln toolchain install` command to run while online; `--no-download` fails without
touching the network but allows a toolchain already in the cache.

## What gets downloaded (linux-x64)

| Id | Used for | Download | On disk |
|---|---|---|---|
| zig 0.17.1 | C, C++ (when no clang), Zig | 55 MB | 407 MB |
| rust 1.100.0 | Rust (`rustc` + wasm32 std) | 178 MB | 503 MB |
| node 22 + assemblyscript 0.28 | AssemblyScript | 57 + 23 MB | 301 MB |
| tinygo 0.44 + go 1.26 | Go | 180 + 64 MB | 1.5 GB |
| kotlin 2.4 + jre 21 | Kotlin | 90 + 50 MB | 253 MB |
| dotnet 10.0.404 + NativeAOT-LLVM + wasi-sdk 29 | C# | 243 + 44 + 114 MB | 1.1 GB |

Only linux-x64 was exercised end to end; the other four platforms use the same code with their own pinned archives,
except C#, which is pinned for linux-x64 alone.
