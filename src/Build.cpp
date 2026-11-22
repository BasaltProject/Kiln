#include "Build.hpp"

#include "Embedded.hpp"
#include "Plugin.hpp"
#include "Process.hpp"
#include "Toolchain.hpp"
#include "Tools.hpp"
#include "WasmCheck.hpp"

#include <basalt/wire/common/toml/Toml.hpp>
#include <basalt/wire/io/FileIO.hpp>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>

namespace fs = std::filesystem;

namespace basalt::kiln {
namespace {

struct Context {
    fs::path root;
    fs::path work;
    fs::path sdk;
    fs::path output;
    std::string id;
    bool proxy = false;
    bool verbose = false;
};

using Outcome = std::expected<fs::path, std::string>;

std::string joined(const std::vector<std::string>& command) {
    std::string text;
    for (const std::string& part : command) {
        text += (text.empty() ? "" : " ") + part;
    }
    return text;
}

std::expected<void, std::string> run(const Context& context, const std::vector<std::string>& command,
                                     const fs::path& cwd, const std::map<std::string, std::string>& environment = {}) {
    if (context.verbose) {
        std::cerr << "kiln: " << joined(command) << '\n';
    }
    const ProcessResult result = runProcess(command, cwd, environment);
    if (!result.started) {
        return std::unexpected("cannot run " + command.front());
    }
    if (result.exitCode != 0) {
        return std::unexpected(result.output + command.front() + " failed with exit code " +
                               std::to_string(result.exitCode));
    }
    if (!result.output.empty()) {
        std::cerr << result.output;
        if (result.output.back() != '\n') {
            std::cerr << '\n';
        }
    }
    return {};
}

std::vector<fs::path> sourcesIn(const fs::path& folder, std::initializer_list<std::string_view> extensions) {
    std::vector<fs::path> found;
    std::error_code error;
    if (!fs::is_directory(folder, error)) {
        return found;
    }
    for (const fs::directory_entry& entry : fs::recursive_directory_iterator(folder, error)) {
        if (entry.is_regular_file() &&
            std::ranges::find(extensions, entry.path().extension().string()) != extensions.end()) {
            found.push_back(entry.path());
        }
    }
    std::ranges::sort(found);
    return found;
}

std::vector<std::string> strings(const std::vector<fs::path>& paths) {
    std::vector<std::string> out;
    for (const fs::path& path : paths) {
        out.push_back(path.string());
    }
    return out;
}

void append(std::vector<std::string>& command, std::initializer_list<std::string> more) {
    command.insert(command.end(), more.begin(), more.end());
}

bool writeIfChanged(const fs::path& path, std::string_view contents) {
    std::ifstream existing{path, std::ios::binary};
    if (existing) {
        const std::string current{std::istreambuf_iterator<char>{existing}, std::istreambuf_iterator<char>{}};
        if (current == contents) {
            return true;
        }
    }
    fs::create_directories(path.parent_path());
    std::ofstream out{path, std::ios::binary | std::ios::trunc};
    out << contents;
    return static_cast<bool>(out);
}

std::expected<void, std::string> extractSdk(const Context& context, std::initializer_list<std::string_view> folders,
                                            bool withAbiHeader = false) {
    for (const std::string_view folder : folders) {
        const std::string prefix = std::string{folder} + "/";
        for (const EmbeddedFile& file : kSdkFiles) {
            const std::string other = context.proxy ? "server" : "proxy";
            if (file.path.starts_with("go/" + other + "/") || file.path.starts_with("go/shared/") ||
                file.path.starts_with("rust/basalt_" + other + "/") ||
                file.path.starts_with("assemblyscript/basalt-" + other + "/") ||
                file.path.starts_with("cpp/basalt/" + other)) {
                continue;
            }
            if (file.path.starts_with(prefix) &&
                !writeIfChanged(context.sdk / std::string{file.path.substr(prefix.size())}, file.contents)) {
                return std::unexpected("cannot write " +
                                       (context.sdk / std::string{file.path.substr(prefix.size())}).string());
            }
        }
    }
    for (const EmbeddedFile& file : withAbiHeader ? kAbiFiles : std::vector<EmbeddedFile>{}) {
        if (!writeIfChanged(context.sdk / std::string{file.path}, file.contents)) {
            return std::unexpected("cannot write " + (context.sdk / std::string{file.path}).string());
        }
    }
    return {};
}

std::expected<void, std::string> need(const std::vector<fs::path>& sources, std::string_view what) {
    if (sources.empty()) {
        return std::unexpected("there are no " + std::string{what} + " files under src/");
    }
    return {};
}

fs::path cleaned(const fs::path& path) {
    fs::path normal = fs::absolute(path).lexically_normal();
    return normal.has_filename() ? normal : normal.parent_path();
}

std::expected<void, std::string> extractCSdk(const Context& context) {
    const std::string other = context.proxy ? "server" : "proxy";
    for (const EmbeddedFile& file : kSdkFiles) {
        if (!file.path.starts_with("c/")) {
            continue;
        }
        const std::string_view relative = file.path.substr(2);
        if (relative.starts_with("basalt/" + other + "/") || relative == "basalt/" + other + ".h" ||
            relative == "runtime/" + other + ".c") {
            continue;
        }
        if (!writeIfChanged(context.sdk / std::string{relative}, file.contents)) {
            return std::unexpected("cannot write " + (context.sdk / std::string{relative}).string());
        }
    }
    return extractSdk(context, {}, true);
}

Outcome buildC(const Context& context, bool cpp) {
    std::vector<fs::path> sources =
        cpp ? sourcesIn(context.root / "src", {".cpp", ".cc", ".cxx"}) : sourcesIn(context.root / "src", {".c"});
    if (const auto missing = need(sources, cpp ? "C++" : "C"); !missing.has_value()) {
        return std::unexpected(missing.error());
    }
    if (const auto written = extractCSdk(context); !written.has_value()) {
        return std::unexpected(written.error());
    }
    if (cpp) {
        Context cppContext = context;
        cppContext.sdk = context.sdk / "basalt";
        if (const auto written = extractSdk(cppContext, {"cpp/basalt"}); !written.has_value()) {
            return std::unexpected(written.error());
        }
    }
    const auto compiler = findCCompiler();
    if (!compiler.has_value()) {
        return std::unexpected(compiler.error());
    }
    const auto driver = [&](bool plusPlus) {
        std::vector<std::string> command{compiler->tool.executable.string()};
        if (compiler->zig) {
            append(command, {plusPlus ? "c++" : "cc", "-target", "wasm32-freestanding"});
        } else {
            append(command, {"--target=wasm32", plusPlus ? "--driver-mode=g++" : "--driver-mode=gcc"});
        }
        append(command, {"-nostdlib", "-ffreestanding", "-O2", "-mbulk-memory", "-I" + context.sdk.string()});
        return command;
    };

    const std::vector<fs::path> runtimeSources = std::vector<fs::path>{
        context.sdk / "runtime" / "shared.c", context.sdk / "runtime" / (context.proxy ? "proxy.c" : "server.c")};
    std::vector<std::string> runtimeObjects;
    for (const fs::path& source : runtimeSources) {
        const fs::path runtimeObject = context.work / (source.stem().string() + "_runtime.o");
        std::vector<std::string> runtime = driver(false);
        append(runtime, {"-c", source.string(), "-o", runtimeObject.string(), "-Wno-comment"});
        if (const auto done = run(context, runtime, context.root, compiler->tool.environment); !done.has_value()) {
            return std::unexpected(done.error());
        }
        runtimeObjects.push_back(runtimeObject.string());
    }

    std::vector<std::string> link = driver(cpp);
    append(link, {"-I" + (context.root / "src").string()});
    if (cpp) {
        append(link, {"-std=c++23", "-fno-exceptions", "-fno-rtti", "-fno-threadsafe-statics"});
    }
    if (!compiler->zig) {
        link.push_back("-fuse-ld=lld");
    }
    append(link, {"-Wl,--no-entry", "-Wl,-z,stack-size=131072", "-Wl,--strip-all", "-Wno-comment"});
    for (const std::string& source : strings(sources)) {
        link.push_back(source);
    }
    link.insert(link.end(), runtimeObjects.begin(), runtimeObjects.end());
    append(link, {"-o", context.output.string()});
    if (const auto done = run(context, link, context.root, compiler->tool.environment); !done.has_value()) {
        return std::unexpected(done.error());
    }
    return context.output;
}

Outcome buildZig(const Context& context) {
    const fs::path main = context.root / "src" / "main.zig";
    if (!fs::is_regular_file(main)) {
        return std::unexpected("src/main.zig is missing; it must define `pub fn init() void`");
    }
    const std::string sdkModule = context.proxy ? "basalt_proxy" : "basalt_server";
    std::error_code ignored;
    fs::remove_all(context.sdk / (context.proxy ? "basalt_server" : "basalt_proxy"), ignored);
    for (const std::string& module : {std::string{"basalt_shared"}, sdkModule}) {
        Context moduleContext = context;
        moduleContext.sdk = context.sdk / module;
        if (const auto written = extractSdk(moduleContext, {"zig/" + module}); !written.has_value()) {
            return std::unexpected(written.error());
        }
    }
    const fs::path entry = context.work / "root.zig";
    if (!writeIfChanged(entry, "const basalt = @import(\"" + sdkModule +
                                   "\");\nconst plugin = @import(\"plugin\");\n\ncomptime {\n    "
                                   "basalt.core.install(plugin.init);\n}\n")) {
        return std::unexpected("cannot write " + entry.string());
    }
    const auto zig = findZig();
    if (!zig.has_value()) {
        return std::unexpected(zig.error());
    }
    const std::vector<std::string> command{zig->executable.string(),
                                           "build-exe",
                                           "-target",
                                           "wasm32-freestanding",
                                           "-O",
                                           "ReleaseSmall",
                                           "-fno-entry",
                                           "-rdynamic",
                                           "--cache-dir",
                                           (context.work / "zig-cache").string(),
                                           "--dep",
                                           "plugin",
                                           "--dep",
                                           sdkModule,
                                           "-Mroot=" + entry.string(),
                                           "--dep",
                                           sdkModule,
                                           "-Mplugin=" + main.string(),
                                           "--dep",
                                           "basalt_shared",
                                           "-M" + sdkModule + "=" + (context.sdk / sdkModule / "root.zig").string(),
                                           "-Mbasalt_shared=" + (context.sdk / "basalt_shared" / "root.zig").string(),
                                           "-femit-bin=" + context.output.string()};
    if (const auto done = run(context, command, context.root, zig->environment); !done.has_value()) {
        return std::unexpected(done.error());
    }
    return context.output;
}

Outcome buildRust(const Context& context) {
    const fs::path lib = context.root / "src" / "lib.rs";
    if (!fs::is_regular_file(lib)) {
        return std::unexpected("src/lib.rs is missing; it must define `pub fn init()`");
    }
    if (const auto written = extractSdk(context, {"rust"}); !written.has_value()) {
        return std::unexpected(written.error());
    }
    const std::string sdkCrate = context.proxy ? "basalt_proxy" : "basalt_server";
    std::string escaped = fs::absolute(lib).generic_string();
    const std::string entryText = "extern crate " + sdkCrate + " as basalt;\n#[path = \"" + escaped +
                                  "\"]\nmod plugin;\n#[no_mangle]\npub extern \"C\" fn basalt_abi_version() -> i32 { "
                                  "basalt::core::ABI_VERSION }\n"
                                  "#[no_mangle]\npub extern \"C\" fn basalt_init() { plugin::init(); }\n"
                                  "#[no_mangle]\npub extern \"C\" fn basalt_callback(token: i32, ctx: i32) { "
                                  "basalt::core::dispatch(token, ctx); }\n";
    const fs::path entry = context.work / "entry.rs";
    if (!writeIfChanged(entry, entryText)) {
        return std::unexpected("cannot write " + entry.string());
    }
    const auto rustc = findRust();
    if (!rustc.has_value()) {
        return std::unexpected(rustc.error());
    }
    const std::string exe = rustc->executable.string();
    const std::string rlib = (context.work / "libbasalt.rlib").string();
    const std::vector<std::string> common{"--edition", "2021",        "--target", "wasm32-unknown-unknown",
                                          "-C",        "opt-level=s", "-C",       "panic=abort"};
    std::vector<std::string> sdk{exe};
    sdk.insert(sdk.end(), common.begin(), common.end());
    append(sdk, {"--crate-type", "rlib", "--crate-name", sdkCrate, (context.sdk / sdkCrate / "lib.rs").string(), "-o",
                 rlib});
    if (const auto done = run(context, sdk, context.root, rustc->environment); !done.has_value()) {
        return std::unexpected(done.error());
    }
    std::vector<std::string> plugin{exe};
    plugin.insert(plugin.end(), common.begin(), common.end());
    append(plugin,
           {"--crate-type", "cdylib", "--crate-name", "plugin", "-C", "strip=symbols", "-C", "lto=fat", "--extern",
            sdkCrate + "=" + rlib, "-L", context.work.string(), entry.string(), "-o", context.output.string()});
    if (const auto done = run(context, plugin, context.root, rustc->environment); !done.has_value()) {
        return std::unexpected(done.error());
    }
    return context.output;
}

Outcome buildAssemblyScript(const Context& context) {
    if (!fs::is_regular_file(context.root / "src" / "main.ts")) {
        return std::unexpected("src/main.ts is missing; it must `export function init(): void`");
    }
    if (const auto written = extractSdk(context, {"assemblyscript"}); !written.has_value()) {
        return std::unexpected(written.error());
    }
    const fs::path entry = context.work / "entry.ts";
    const std::string mainImport = (fs::relative(context.root / "src" / "main", context.work)).generic_string();
    const std::string entryText =
        "import { dispatch } from \"basalt-" + std::string{context.proxy ? "proxy" : "server"} +
        "/core\";\nimport { init } from \"" + mainImport +
        "\";\n\n"
        "// @ts-ignore: decorator\n@global function abort(message: usize, file: usize, line: u32, column: u32): void { "
        "unreachable(); }\n\n"
        "export function basalt_abi_version(): i32 { return 1; }\nexport function basalt_init(): void { init(); }\n"
        "export function basalt_callback(token: i32, ctx: i32): void { dispatch(token, ctx); }\n";
    if (!writeIfChanged(entry, entryText)) {
        return std::unexpected("cannot write " + entry.string());
    }
    const auto node = findNode();
    if (!node.has_value()) {
        return std::unexpected(node.error());
    }
    const auto asc = findAssemblyScript(context.root);
    if (!asc.has_value()) {
        return std::unexpected(asc.error());
    }
    const std::vector<std::string> command{node->executable.string(),
                                           asc->string(),
                                           entry.string(),
                                           "--path",
                                           context.sdk.string(),
                                           "--outFile",
                                           context.output.string(),
                                           "--optimizeLevel",
                                           "3",
                                           "--shrinkLevel",
                                           "2",
                                           "--noAssert",
                                           "--runtime",
                                           "incremental",
                                           "--use",
                                           "abort="};
    if (const auto done = run(context, command, context.root, node->environment); !done.has_value()) {
        return std::unexpected(done.error());
    }
    return context.output;
}

Outcome buildGo(const Context& context) {
    if (!fs::is_regular_file(context.root / "go.mod")) {
        return std::unexpected(
            "go.mod is missing; the module must import basaltmc.org/sdk/server or basaltmc.org/sdk/proxy");
    }
    if (const auto written = extractSdk(context, {"go"}); !written.has_value()) {
        return std::unexpected(written.error());
    }
    const std::string workText = "go 1.22\n\nuse (\n\t" + fs::absolute(context.root).generic_string() + "\n\t" +
                                 fs::absolute(context.sdk).generic_string() + "\n)\n";
    const fs::path workFile = context.work / "go.work";
    if (!writeIfChanged(workFile, workText)) {
        return std::unexpected("cannot write " + workFile.string());
    }
    const auto tinygo = findTinyGo();
    if (!tinygo.has_value()) {
        return std::unexpected(tinygo.error());
    }
    std::map<std::string, std::string> environment = tinygo->environment;
    environment["GOWORK"] = fs::absolute(workFile).string();
    environment["GOFLAGS"] = "-buildvcs=false";
    const std::vector<std::string> command{tinygo->executable.string(), "build",  "-target",   "wasm-unknown",
                                           "-scheduler=none",           "-opt=z", "-no-debug", "-o",
                                           context.output.string(),     "."};
    if (const auto done = run(context, command, context.root, environment); !done.has_value()) {
        return std::unexpected(done.error());
    }
    return context.output;
}

Outcome buildKotlin(const Context& context) {
    const std::vector<fs::path> sources = sourcesIn(context.root / "src", {".kt"});
    if (const auto missing = need(sources, "Kotlin"); !missing.has_value()) {
        return std::unexpected(missing.error());
    }
    if (const auto written = extractSdk(context, {context.proxy ? "kotlin/proxy" : "kotlin/server"});
        !written.has_value()) {
        return std::unexpected(written.error());
    }
    std::string kotlinEntry = "init";
    for (const fs::path& source : sources) {
        const auto content = wire::io::fileIO::readText(source.string());
        if (content.has_value() && content->find("fun init()") != std::string::npos) {
            const auto start = content->find("package ");
            if (start != std::string::npos) {
                const auto end = content->find_first_of("\r\n;", start);
                kotlinEntry = content->substr(start + 8, end - start - 8) + ".init";
            }
        }
    }
    const std::string entryText =
        "@file:OptIn(kotlin.wasm.ExperimentalWasmInterop::class)\nimport kotlin.wasm.WasmExport\n\n"
        "@WasmExport(\"basalt_abi_version\")\nfun basaltAbiVersion(): Int = 1\n\n"
        "@WasmExport(\"basalt_init\")\nfun basaltInit() = " +
        kotlinEntry +
        "()\n\n"
        "@WasmExport(\"basalt_callback\")\nfun basaltCallback(token: Int, ctx: Int) = org.basaltmc." +
        (context.proxy ? "proxy" : "server") + ".core.dispatch(token, ctx)\n";
    const fs::path entry = context.work / "Entry.kt";
    if (!writeIfChanged(entry, entryText)) {
        return std::unexpected("cannot write " + entry.string());
    }
    const auto kotlin = findKotlin();
    if (!kotlin.has_value()) {
        return std::unexpected(kotlin.error());
    }
    const auto java = findJava();
    if (!java.has_value()) {
        return std::unexpected(java.error());
    }
    const fs::path lib = *kotlin / "lib";
    const std::string stdlib = (lib / "kotlin-stdlib-wasm-wasi.klib").string();
    const std::string classpath = (lib / "kotlin-compiler.jar").string();
    const auto kotlinc = [&](std::vector<std::string> arguments) {
        std::vector<std::string> command{java->executable.string(),
                                         "-cp",
                                         (lib / "kotlin-preloader.jar").string(),
                                         "org.jetbrains.kotlin.preloading.Preloader",
                                         "-cp",
                                         classpath,
                                         "org.jetbrains.kotlin.cli.js.K2JSCompiler",
                                         "-Xwasm",
                                         "-Xwasm-target=wasm-wasi",
                                         "-nowarn"};
        command.insert(command.end(), arguments.begin(), arguments.end());
        return command;
    };
    const fs::path klibDir = context.work / "klib";
    const fs::path outDir = context.work / "out";
    std::vector<std::string> compile =
        kotlinc({"-Xir-produce-klib-file", "-Xir-module-name=plugin", "-libraries=" + stdlib, "-ir-output-dir",
                 klibDir.string(), "-ir-output-name", "plugin", entry.string()});
    for (const fs::path& source : sourcesIn(context.sdk, {".kt"})) {
        compile.push_back(source.string());
    }
    for (const std::string& source : strings(sources)) {
        compile.push_back(source);
    }
    if (const auto done = run(context, compile, context.root, java->environment); !done.has_value()) {
        return std::unexpected(done.error());
    }
    const std::vector<std::string> link =
        kotlinc({"-Xir-produce-js", "-Xinclude=" + (klibDir / "plugin.klib").string(), "-libraries=" + stdlib,
                 "-ir-output-dir", outDir.string(), "-ir-output-name", "plugin"});
    if (const auto done = run(context, link, context.root, java->environment); !done.has_value()) {
        return std::unexpected(done.error());
    }
    std::error_code error;
    fs::copy_file(outDir / "plugin.wasm", context.output, fs::copy_options::overwrite_existing, error);
    if (error) {
        return std::unexpected("the Kotlin compiler did not produce " + (outDir / "plugin.wasm").string());
    }
    return context.output;
}

Outcome buildCSharp(const Context& context) {
    const std::vector<fs::path> sources = sourcesIn(context.root / "src", {".cs"});
    if (const auto missing = need(sources, "C#"); !missing.has_value()) {
        return std::unexpected(missing.error());
    }
    if (const auto written = extractSdk(context, {context.proxy ? "csharp/proxy" : "csharp/server", "csharp/runtime"});
        !written.has_value()) {
        return std::unexpected(written.error());
    }
    if (currentPlatform() != "linux-x64") {
        return std::unexpected("C# builds only on linux-x64: NativeAOT-LLVM ships its compiler for no other platform "
                               "that Kiln pins yet");
    }
    const auto dotnet = ensureToolchain("dotnet");
    if (!dotnet.has_value()) {
        return std::unexpected(dotnet.error());
    }
    const auto wasiSdk = ensureToolchain("wasisdk");
    if (!wasiSdk.has_value()) {
        return std::unexpected(wasiSdk.error());
    }
    std::string pluginClass = "Plugin";
    for (const fs::path& source : sources) {
        const auto content = wire::io::fileIO::readText(source.string());
        if (content.has_value() && content->find("class Plugin") != std::string::npos) {
            const auto start = content->find("namespace ");
            if (start != std::string::npos) {
                const auto end = content->find_first_of(";{\r\n", start);
                pluginClass = content->substr(start + 10, end - start - 10) + ".Plugin";
            }
        }
    }
    const fs::path entry = context.work / "Entry.cs";
    if (!writeIfChanged(
            entry, "using System.Runtime.InteropServices;\npublic static class BasaltEntry {\n"
                   "[UnmanagedCallersOnly(EntryPoint = \"basalt_abi_version\")] public static int Version() => 1;\n"
                   "[UnmanagedCallersOnly(EntryPoint = \"basalt_init\")] public static void Init() => " +
                       pluginClass +
                       ".Init();\n"
                       "[UnmanagedCallersOnly(EntryPoint = \"basalt_callback\")] public static void Callback(int "
                       "token, int ctx) => Basalt." +
                       std::string{context.proxy ? "Proxy" : "Server"} +
                       ".Internal.Native.Dispatch(token, ctx);\n}\n")) {
        return std::unexpected("cannot write " + entry.string());
    }
    const fs::path packages = *dotnet / "nuget";
    std::error_code error;
    for (const fs::directory_entry& id : fs::directory_iterator(packages, error)) {
        for (const fs::directory_entry& version : fs::directory_iterator(id.path(), error)) {
            if (!writeIfChanged(version.path() / ".nupkg.metadata",
                                "{\"version\": 2, \"contentHash\": \"\", \"source\": \"\"}")) {
                return std::unexpected("cannot write into " + version.path().string());
            }
            for (const fs::directory_entry& file : fs::recursive_directory_iterator(version.path(), error)) {
                if (file.path().extension() == ".wit") {
                    fs::remove(file.path(), error);
                }
            }
        }
    }
    const fs::path feed = context.work / "feed";
    fs::create_directories(feed, error);
    const std::string nugetConfig =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<configuration>\n  <packageSources>\n    <clear />\n"
        "    <add key=\"offline\" value=\"" +
        feed.string() + "\" />\n  </packageSources>\n</configuration>\n";
    const std::string ilcompiler = "10.0.0-preview.2.25509.1";
    std::string project =
        "<Project Sdk=\"Microsoft.NET.Sdk\">\n  <PropertyGroup>\n    <OutputType>library</OutputType>\n"
        "    <TargetFramework>net10.0</TargetFramework>\n    <RuntimeIdentifier>wasi-wasm</RuntimeIdentifier>\n"
        "    <AssemblyName>plugin</AssemblyName>\n    <AllowUnsafeBlocks>true</AllowUnsafeBlocks>\n"
        "    <PublishTrimmed>true</PublishTrimmed>\n    <SelfContained>true</SelfContained>\n"
        "    <MSBuildEnableWorkloadResolver>false</MSBuildEnableWorkloadResolver>\n"
        "    <InvariantGlobalization>true</InvariantGlobalization>\n    <DebugType>none</DebugType>\n"
        "    <EnableDefaultItems>false</EnableDefaultItems>\n    <ImplicitUsings>disable</ImplicitUsings>\n"
        "    <Nullable>disable</Nullable>\n"
        "  </PropertyGroup>\n  <ItemGroup>\n"
        "    <PackageReference Include=\"Microsoft.DotNet.ILCompiler.LLVM\" Version=\"" +
        ilcompiler +
        "\" />\n"
        "    <PackageReference Include=\"runtime.linux-x64.Microsoft.DotNet.ILCompiler.LLVM\" Version=\"" +
        ilcompiler + "\" />\n    <Compile Include=\"" + entry.string() + "\" />\n";
    for (const fs::path& source : sourcesIn(context.sdk, {".cs"})) {
        project += "    <Compile Include=\"" + source.string() + "\" />\n";
    }
    for (const fs::path& source : sources) {
        project += "    <Compile Include=\"" + source.string() + "\" />\n";
    }
    project +=
        "    <LinkerArg Include=\"" + (context.sdk / "wasi_stubs.c").string() + "\" />\n  </ItemGroup>\n</Project>\n";
    if (!writeIfChanged(context.work / "plugin.csproj", project) ||
        !writeIfChanged(context.work / "nuget.config", nugetConfig)) {
        return std::unexpected("cannot write the project in " + context.work.string());
    }
    const fs::path out = context.work / "out";
    fs::remove_all(out, error);
    const std::map<std::string, std::string> environment{
        {"DOTNET_ROOT", dotnet->string()},          {"DOTNET_CLI_HOME", (context.work / "home").string()},
        {"DOTNET_CLI_TELEMETRY_OPTOUT", "1"},       {"DOTNET_NOLOGO", "1"},
        {"DOTNET_SKIP_FIRST_TIME_EXPERIENCE", "1"}, {"DOTNET_MULTILEVEL_LOOKUP", "0"},
        {"NUGET_PACKAGES", packages.string()},      {"WASI_SDK_PATH", wasiSdk->string() + "/"},
    };
    const std::vector<std::string> command{(*dotnet / executableName("dotnet")).string(),
                                           "publish",
                                           (context.work / "plugin.csproj").string(),
                                           "-c",
                                           "Release",
                                           "-r",
                                           "wasi-wasm",
                                           "-o",
                                           out.string(),
                                           "-p:IlcLlvmTarget=wasm32-unknown-wasip1",
                                           "-nodeReuse:false",
                                           "-clp:ErrorsOnly",
                                           "--configfile",
                                           (context.work / "nuget.config").string()};
    if (const auto done = run(context, command, context.work, environment); !done.has_value()) {
        return std::unexpected(done.error());
    }
    fs::copy_file(out / "plugin.wasm", context.output, fs::copy_options::overwrite_existing, error);
    if (error) {
        return std::unexpected("the .NET compiler did not produce " + (out / "plugin.wasm").string());
    }
    return context.output;
}

Outcome buildJava(const Context& context) {
    const std::vector<fs::path> sources = sourcesIn(context.root / "src", {".java"});
    if (const auto missing = need(sources, "Java"); !missing.has_value()) {
        return std::unexpected(missing.error());
    }
    if (const auto written = extractSdk(context, {context.proxy ? "java/proxy" : "java/server", "java/glue"}, true);
        !written.has_value()) {
        return std::unexpected(written.error());
    }
    std::string entryClass = "Main";
    for (const fs::path& source : sources) {
        if (source.filename() == "Main.java") {
            const auto content = wire::io::fileIO::readText(source.string());
            if (content.has_value()) {
                const auto start = content->find("package ");
                if (start != std::string::npos) {
                    const auto end = content->find(';', start);
                    entryClass = content->substr(start + 8, end - start - 8) + ".Main";
                }
            }
        }
    }
    const fs::path entry = context.work / "gen" / "BasaltEntry.java";
    const fs::path services = context.work / "services" / "META-INF" / "services" / "org.teavm.vm.spi.TeaVMPlugin";
    if (!writeIfChanged(entry, "public final class BasaltEntry {\n    public static void main(String[] args) {\n"
                               "        " +
                                   entryClass + ".init();\n        org.basaltmc." +
                                   (context.proxy ? "proxy" : "server") +
                                   ".internal.Callbacks.install();\n    }\n}\n") ||
        !writeIfChanged(services, "org.teavm.platform.plugin.PlatformPlugin\norg.teavm.jso.impl.JSOPlugin\n"
                                  "org.teavm.metaprogramming.impl.MetaprogrammingPlugin\n"
                                  "org.teavm.classlib.impl.JCLPlugin\n")) {
        return std::unexpected("cannot write the generated Java files under " + context.work.string());
    }
    const auto tools = findTeaVm();
    if (!tools.has_value()) {
        return std::unexpected(tools.error());
    }
    const auto zig = findZig();
    if (!zig.has_value()) {
        return std::unexpected(zig.error());
    }
    const fs::path classes = context.work / "classes";
    const fs::path generatedC = context.work / "c";
    std::error_code error;
    fs::remove_all(classes, error);
    fs::remove_all(generatedC, error);
    fs::create_directories(classes, error);

    std::vector<std::string> compile{
        tools->java.executable.string(), "-jar", tools->ecj.string(), "-17", "-nowarn", "-cp",
        tools->teavm.string(),           "-d",   classes.string()};
    for (const std::string& source : strings(sources)) {
        compile.push_back(source);
    }
    for (const std::string& source : strings(sourcesIn(context.sdk / "org", {".java"}))) {
        compile.push_back(source);
    }
    compile.push_back(entry.string());
    if (const auto done = run(context, compile, context.root, tools->java.environment); !done.has_value()) {
        return std::unexpected(done.error());
    }

#if defined(_WIN32)
    constexpr char kPathSeparator = ';';
#else
    constexpr char kPathSeparator = ':';
#endif
    const std::string classpath = (context.work / "services").string() + kPathSeparator + tools->teavm.string() +
                                  kPathSeparator + classes.string();
    const std::vector<std::string> translate{tools->java.executable.string(),
                                             "-cp",
                                             classpath,
                                             "org.teavm.cli.TeaVMRunner",
                                             "-t",
                                             "C",
                                             "-d",
                                             generatedC.string(),
                                             "-O",
                                             "3",
                                             "--min-heap",
                                             "1",
                                             "--max-heap",
                                             "16",
                                             "BasaltEntry"};
    if (const auto done = run(context, translate, context.root, tools->java.environment); !done.has_value()) {
        return std::unexpected(done.error());
    }

    std::vector<std::string> link{zig->executable.string(), "cc", "-target", "wasm32-wasi", "-O2"};
    append(link, {"-mexec-model=reactor",
                  "-mexception-handling",
                  "-mllvm",
                  "-wasm-enable-sjlj",
                  "-mllvm",
                  "-wasm-use-legacy-eh=false",
                  "-D__EMSCRIPTEN__",
                  "-Dgetuid()=0",
                  "-Dmain=teavm_boot_main",
                  "-D_WASI_EMULATED_SIGNAL",
                  "-D_WASI_EMULATED_MMAN",
                  "-D_WASI_EMULATED_PROCESS_CLOCKS",
                  "-w",
                  "-Wno-incompatible-pointer-types",
                  "-I" + context.sdk.string(),
                  "-iquote",
                  generatedC.string(),
                  (context.sdk / "basalt_glue.c").string(),
                  "-o",
                  context.output.string(),
                  "-lwasi-emulated-signal",
                  "-lwasi-emulated-mman",
                  "-lwasi-emulated-process-clocks",
                  "-Wl,--export-memory",
                  "-Wl,--strip-all"});
    if (const auto done = run(context, link, context.root, zig->environment); !done.has_value()) {
        return std::unexpected(done.error());
    }
    return context.output;
}

} // namespace

std::expected<fs::path, std::string> buildPlugin(const fs::path& root, const BuildOptions& options) {
    const std::optional<std::string> text = wire::io::fileIO::readText((root / "plugin.toml").string());
    if (!text.has_value()) {
        return std::unexpected("there is no plugin.toml in " + root.string());
    }
    const auto table = wire::common::toml::parse(*text);
    if (!table.has_value()) {
        return std::unexpected("plugin.toml: " + std::string{wire::common::toml::tomlErrorMessage(table.error())});
    }
    const std::string main{table->string("main").value_or("plugin.wasm")};
    if (!isWasmMain(main)) {
        return std::unexpected("plugin.toml: `main` must name a .wasm module");
    }
    std::optional<Language> language = options.language;
    if (!language.has_value()) {
        const auto declared = table->string("lang");
        language = detectLanguage(root, declared);
        if (!language.has_value()) {
            return std::unexpected(declared.has_value()
                                       ? "plugin.toml: `lang` is not one of " + languageList()
                                       : "cannot tell the language from the files in " + root.string() +
                                             "; set `lang` in plugin.toml to one of " + languageList());
        }
    }
    const fs::path build = cleaned(root) / "build";
    Context context{.root = cleaned(root),
                    .work = build / ".kiln" / std::string{infoOf(*language).name},
                    .sdk = build / ".kiln" / "sdk" / std::string{infoOf(*language).name},
                    .output = build / fs::path{main}.filename(),
                    .id = std::string{table->string("id").value_or("")},
                    .proxy = options.target.value_or(manifestTarget(root)) == "proxy",
                    .verbose = options.verbose};
    std::error_code error;
    fs::create_directories(context.work, error);
    fs::remove_all(context.sdk, error);
    if (error) {
        return std::unexpected("cannot refresh " + context.sdk.string() + ": " + error.message());
    }
    fs::create_directories(context.sdk, error);
    if (error) {
        return std::unexpected("cannot create " + context.work.string() + ": " + error.message());
    }
    fs::remove(context.output, error);

    const auto started = std::chrono::steady_clock::now();
    Outcome built = std::unexpected("unreachable");
    switch (*language) {
    case Language::C:
        built = buildC(context, false);
        break;
    case Language::Cpp:
        built = buildC(context, true);
        break;
    case Language::Zig:
        built = buildZig(context);
        break;
    case Language::Rust:
        built = buildRust(context);
        break;
    case Language::AssemblyScript:
        built = buildAssemblyScript(context);
        break;
    case Language::Go:
        built = buildGo(context);
        break;
    case Language::Kotlin:
        built = buildKotlin(context);
        break;
    case Language::CSharp:
        built = buildCSharp(context);
        break;
    case Language::Java:
        built = buildJava(context);
        break;
    }
    if (!built.has_value()) {
        return built;
    }
    const auto bytes = wire::io::fileIO::readBytes(built->string());
    if (!bytes.has_value()) {
        return std::unexpected("the build did not produce " + built->string());
    }
    const std::vector<Diagnostic> problems = verifyModule(*bytes, fs::relative(*built, context.root).generic_string(),
                                                          options.target.value_or(manifestTarget(root)));
    if (!problems.empty()) {
        std::string report;
        for (const Diagnostic& problem : problems) {
            report += problem.file + ": " + problem.message + "\n";
        }
        return std::unexpected(report + "the module does not follow the plugin ABI");
    }
    const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
    std::cerr << "kiln: built " << fs::relative(*built, context.root).generic_string() << " (" << bytes->size()
              << " bytes, " << infoOf(*language).title << ", " << static_cast<int>(seconds * 10 + 0.5) / 10.0
              << " s)\n";
    return built;
}

} // namespace basalt::kiln
