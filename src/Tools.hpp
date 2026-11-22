#pragma once

#include <expected>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace basalt::kiln {

/// A program to run plus the environment that makes it self-contained.
struct Tool {
    std::filesystem::path executable;
    std::map<std::string, std::string> environment;
};

/// An executable inside an installed toolchain, looking at `bin/` and the macOS bundle layout.
[[nodiscard]] std::filesystem::path executableIn(const std::filesystem::path& toolchain, const std::string& name);

/// A system `clang` or `zig` that can build wasm32, else the pinned Zig; `zig` says which one was found.
struct CCompiler {
    Tool tool;
    bool zig = false;
};

[[nodiscard]] std::expected<CCompiler, std::string> findCCompiler();

[[nodiscard]] std::expected<Tool, std::string> findZig();
[[nodiscard]] std::expected<Tool, std::string> findRust();
[[nodiscard]] std::expected<Tool, std::string> findNode();
[[nodiscard]] std::expected<Tool, std::string> findJava();
/// TinyGo, with the Go toolchain it needs on `PATH` and as `GOROOT` when it is not the system one.
[[nodiscard]] std::expected<Tool, std::string> findTinyGo();

/// The AssemblyScript compiler script: the plugin's own `node_modules`, else the pinned one.
[[nodiscard]] std::expected<std::filesystem::path, std::string> findAssemblyScript(const std::filesystem::path& root);

/// The folder of the pinned Kotlin compiler.
[[nodiscard]] std::expected<std::filesystem::path, std::string> findKotlin();

/// A Java runtime with the pinned TeaVM and ecj jars, which compile Java to C and bytecode.
struct JavaTools {
    Tool java;
    std::filesystem::path teavm;
    std::filesystem::path ecj;
};

[[nodiscard]] std::expected<JavaTools, std::string> findTeaVm();

} // namespace basalt::kiln
