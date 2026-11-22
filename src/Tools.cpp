#include "Tools.hpp"

#include "Process.hpp"
#include "Toolchain.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <optional>

namespace fs = std::filesystem;

namespace basalt::kiln {
namespace {

std::map<std::string, std::string> cacheEnvironment(const fs::path& scratch) {
    return {{"ZIG_GLOBAL_CACHE_DIR", (scratch / "zig-global").string()},
            {"ZIG_LOCAL_CACHE_DIR", (scratch / "zig-local").string()},
            {"GOCACHE", (scratch / "go-build").string()},
            {"XDG_CACHE_HOME", (scratch / "xdg").string()}};
}

fs::path scratchDirectory() { return cacheSettings().root / "work"; }

std::optional<std::pair<int, int>> versionOf(const std::string& text) {
    int major = 0;
    int minor = 0;
    std::size_t at = text.find_first_of("0123456789");
    if (at == std::string::npos) {
        return std::nullopt;
    }
    major = std::atoi(text.c_str() + at);
    const std::size_t dot = text.find('.', at);
    if (dot != std::string::npos) {
        minor = std::atoi(text.c_str() + dot + 1);
    }
    return std::pair{major, minor};
}

bool atLeast(const std::string& text, int major, int minor) {
    const auto version = versionOf(text);
    return version.has_value() && (version->first > major || (version->first == major && version->second >= minor));
}

bool wasmClang(const fs::path& clang) {
    const fs::path probeDir = scratchDirectory() / "probe";
    std::error_code error;
    fs::create_directories(probeDir, error);
    {
        std::ofstream source{probeDir / "probe.c"};
        source << "int probe(void) { return 1; }\n";
    }
    const ProcessResult result =
        runProcess({clang.string(), "--target=wasm32", "-nostdlib", "-ffreestanding", "-fuse-ld=lld", "-Wl,--no-entry",
                    "-Wl,--export-all", "-o", (probeDir / "probe.wasm").string(), (probeDir / "probe.c").string()},
                   probeDir);
    return result.started && result.exitCode == 0;
}

} // namespace

fs::path executableIn(const fs::path& toolchain, const std::string& name) {
    const std::string file = executableName(name);
    std::error_code error;
    for (const fs::path& candidate : {toolchain / "bin" / file, toolchain / file}) {
        if (fs::is_regular_file(candidate, error)) {
            return candidate;
        }
    }
    if (fs::is_directory(toolchain, error)) {
        for (const fs::directory_entry& entry : fs::directory_iterator(toolchain, error)) {
            const fs::path bundled = entry.path() / "Contents" / "Home" / "bin" / file;
            if (fs::is_regular_file(bundled, error)) {
                return bundled;
            }
        }
    }
    return toolchain / "bin" / file;
}

std::expected<Tool, std::string> findZig() {
    if (const auto system = findOnPath("zig"); system.has_value()) {
        const ProcessResult version = runProcess({system->string(), "version"}, fs::current_path());
        if (version.started && version.exitCode == 0 && atLeast(version.output, 0, 14)) {
            return Tool{*system, cacheEnvironment(scratchDirectory())};
        }
    }
    const auto installed = ensureToolchain("zig");
    if (!installed.has_value()) {
        return std::unexpected(installed.error());
    }
    return Tool{executableIn(*installed, "zig"), cacheEnvironment(scratchDirectory())};
}

std::expected<CCompiler, std::string> findCCompiler() {
    for (const char* name : {"clang", "clang-19", "clang-20", "clang-21", "clang-22", "clang-23"}) {
        if (const auto clang = findOnPath(name); clang.has_value() && wasmClang(*clang)) {
            return CCompiler{Tool{*clang, {}}, false};
        }
    }
    const auto zig = findZig();
    if (!zig.has_value()) {
        return std::unexpected("no clang with wasm32 support was found and Zig could not be set up: " + zig.error());
    }
    return CCompiler{*zig, true};
}

std::expected<Tool, std::string> findRust() {
    if (const auto system = findOnPath("rustc"); system.has_value()) {
        const ProcessResult libdir = runProcess(
            {system->string(), "--print", "target-libdir", "--target", "wasm32-unknown-unknown"}, fs::current_path());
        std::string path = libdir.output;
        while (!path.empty() && (path.back() == '\n' || path.back() == '\r')) {
            path.pop_back();
        }
        std::error_code error;
        if (libdir.started && libdir.exitCode == 0 && fs::is_directory(path, error) && !fs::is_empty(path, error)) {
            return Tool{*system, {}};
        }
    }
    const auto installed = ensureToolchain("rust");
    if (!installed.has_value()) {
        return std::unexpected(installed.error());
    }
    return Tool{executableIn(*installed, "rustc"), {}};
}

std::expected<Tool, std::string> findNode() {
    if (const auto system = findOnPath("node"); system.has_value()) {
        const ProcessResult version = runProcess({system->string(), "--version"}, fs::current_path());
        if (version.started && version.exitCode == 0 && atLeast(version.output, 18, 0)) {
            return Tool{*system, {}};
        }
    }
    const auto installed = ensureToolchain("node");
    if (!installed.has_value()) {
        return std::unexpected(installed.error());
    }
    return Tool{executableIn(*installed, "node"), {}};
}

std::expected<Tool, std::string> findJava() {
    if (const auto system = findOnPath("java"); system.has_value()) {
        const ProcessResult version = runProcess({system->string(), "-version"}, fs::current_path());
        const std::size_t quote = version.output.find('"');
        if (version.started && version.exitCode == 0 && quote != std::string::npos &&
            atLeast(version.output.substr(quote), 17, 0)) {
            return Tool{*system, {}};
        }
    }
    const auto installed = ensureToolchain("jre");
    if (!installed.has_value()) {
        return std::unexpected(installed.error());
    }
    return Tool{executableIn(*installed, "java"), {}};
}

std::expected<Tool, std::string> findTinyGo() {
    const auto systemTinyGo = findOnPath("tinygo");
    const auto systemGo = findOnPath("go");
    Tool tool;
    tool.environment = cacheEnvironment(scratchDirectory());
    if (systemTinyGo.has_value() && systemGo.has_value()) {
        tool.executable = *systemTinyGo;
        return tool;
    }
    const auto tinygo = ensureToolchain("tinygo");
    if (!tinygo.has_value()) {
        return std::unexpected(tinygo.error());
    }
    tool.executable = executableIn(*tinygo, "tinygo");
    fs::path goBin;
    if (systemGo.has_value()) {
        goBin = systemGo->parent_path();
    } else {
        const auto go = ensureToolchain("go");
        if (!go.has_value()) {
            return std::unexpected(go.error());
        }
        tool.environment["GOROOT"] = go->string();
        goBin = *go / "bin";
    }
    tool.environment["PATH"] = goBin.string();
    return tool;
}

std::expected<fs::path, std::string> findAssemblyScript(const fs::path& root) {
    const fs::path local = root / "node_modules" / "assemblyscript" / "bin" / "asc.js";
    if (fs::is_regular_file(local)) {
        return local;
    }
    const auto installed = ensureToolchain("assemblyscript");
    if (!installed.has_value()) {
        return std::unexpected(installed.error());
    }
    return *installed / "node_modules" / "assemblyscript" / "bin" / "asc.js";
}

std::expected<fs::path, std::string> findKotlin() { return ensureToolchain("kotlin"); }

std::expected<JavaTools, std::string> findTeaVm() {
    const auto java = findJava();
    if (!java.has_value()) {
        return std::unexpected(java.error());
    }
    const auto installed = ensureToolchain("teavm");
    if (!installed.has_value()) {
        return std::unexpected(installed.error());
    }
    JavaTools tools{*java, {}, {}};
    for (const fs::directory_entry& entry : fs::directory_iterator(*installed)) {
        const std::string name = entry.path().filename().string();
        if (name.starts_with("cli-")) {
            tools.teavm = entry.path();
        } else if (name.starts_with("ecj-")) {
            tools.ecj = entry.path();
        }
    }
    if (tools.teavm.empty() || tools.ecj.empty()) {
        return std::unexpected("the teavm toolchain in " + installed->string() +
                               " is incomplete; run `kiln toolchain clean`");
    }
    return tools;
}

} // namespace basalt::kiln
