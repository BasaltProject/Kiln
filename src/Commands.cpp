#include "Commands.hpp"

#include "Build.hpp"
#include "Languages.hpp"
#include "Pack.hpp"
#include "Plugin.hpp"
#include "Project.hpp"
#include "Scaffold.hpp"
#include "Targets.hpp"
#include "Toolchain.hpp"
#include "WasmCheck.hpp"

#include <basalt/wire/io/FileIO.hpp>

#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

namespace basalt::kiln {
namespace {

constexpr int kUsageError = 2;

void print(const std::vector<Diagnostic>& problems) {
    for (const Diagnostic& problem : problems) {
        std::cerr << problem.file << (problem.line > 0 ? ":" + std::to_string(problem.line) : "") << ": "
                  << problem.message << '\n';
    }
    if (!problems.empty()) {
        std::cerr << problems.size() << " problem(s)\n";
    }
}

struct Parsed {
    fs::path plugin{"."};
    std::optional<Language> language;
    std::optional<std::string> target;
    bool verbose = false;
};

std::optional<Parsed> parse(const std::vector<std::string>& args, bool takesLanguage, bool takesTarget) {
    Parsed parsed;
    std::size_t i = 1;
    if (args.size() > 1 && !args[1].starts_with("-")) {
        parsed.plugin = args[i++];
    }
    for (; i < args.size(); ++i) {
        if (takesLanguage && args[i] == "--lang" && i + 1 < args.size()) {
            parsed.language = languageNamed(args[++i]);
            if (!parsed.language.has_value()) {
                std::cerr << "unknown language " << args[i] << "; use one of " << languageList() << '\n';
                return std::nullopt;
            }
        } else if (takesTarget && args[i] == "--target" && i + 1 < args.size()) {
            const auto named = targetNamed(args[++i]);
            if (!named.has_value()) {
                std::cerr << "unknown target " << args[i] << '\n';
                return std::nullopt;
            }
            parsed.target = std::string{named->name};
        } else if (takesLanguage && (args[i] == "-v" || args[i] == "--verbose")) {
            parsed.verbose = true;
        } else {
            return std::nullopt;
        }
    }
    return parsed;
}

std::string formatBytes(std::uint64_t bytes) {
    if (bytes == 0) {
        return "?";
    }
    return std::to_string((bytes + 512 * 1024) / (1024 * 1024)) + " MB";
}

std::vector<std::string_view> toolchainsFor(std::string_view name) {
    if (const auto language = languageNamed(name); language.has_value()) {
        switch (*language) {
        case Language::C:
        case Language::Cpp:
        case Language::Zig:
            return {"zig"};
        case Language::Rust:
            return {"rust"};
        case Language::AssemblyScript:
            return {"node", "assemblyscript"};
        case Language::Go:
            return {"tinygo", "go"};
        case Language::Kotlin:
            return {"kotlin", "jre"};
        case Language::Java:
            return {"teavm", "jre", "zig"};
        case Language::CSharp:
            return {};
        }
    }
    const std::vector<std::string_view> ids = toolchainIds();
    if (std::ranges::find(ids, name) != ids.end()) {
        return {*std::ranges::find(ids, name)};
    }
    return {};
}

} // namespace

int runNew(const std::vector<std::string>& args) {
    std::vector<std::string> positional;
    Language language = Language::Cpp;
    std::string target = "server";
    for (std::size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "--lang" && i + 1 < args.size()) {
            const auto named = languageNamed(args[++i]);
            if (!named.has_value()) {
                std::cerr << "unknown language " << args[i] << "; use one of " << languageList() << '\n';
                return kUsageError;
            }
            language = *named;
        } else if (args[i] == "--target" && i + 1 < args.size()) {
            if (!targetNamed(args[++i]).has_value()) {
                std::cerr << "unknown target " << args[i] << "; use server or proxy\n";
                return kUsageError;
            }
            target = args[i];
        } else {
            positional.push_back(args[i]);
        }
    }
    if (positional.empty() || positional.size() > 2) {
        std::cerr << "usage: kiln new <id> [name] [--lang " << languageList() << "] [--target server|proxy]\n";
        return kUsageError;
    }
    if (const std::string failure =
            scaffold(".", positional[0], positional.size() > 1 ? positional[1] : positional[0], language, target);
        !failure.empty()) {
        std::cerr << "kiln new: " << failure << '\n';
        return 1;
    }
    std::cout << "Created " << positional[0] << "/ (" << infoOf(language).title << ")\n";
    return 0;
}

int runBuild(const std::vector<std::string>& args) {
    const std::optional<Parsed> parsed = parse(args, true, true);
    if (!parsed.has_value()) {
        std::cerr << "usage: kiln build [plugin] [--lang <language>] [--target server|proxy] [-v]\n";
        return kUsageError;
    }
    const auto built =
        buildPlugin(parsed->plugin,
                    BuildOptions{.language = parsed->language, .target = parsed->target, .verbose = parsed->verbose});
    if (!built.has_value()) {
        std::cerr << "kiln build: " << built.error() << '\n';
        return 1;
    }
    return 0;
}

int runCheck(const std::vector<std::string>& args) {
    std::optional<std::string> target;
    std::optional<fs::path> given;
    bool usage = false;
    for (std::size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "--target" && i + 1 < args.size() && targetNamed(args[i + 1]).has_value()) {
            target = args[++i];
        } else if (args[i] != "--no-module" && !args[i].starts_with("-") && !given.has_value()) {
            given = args[i];
        } else if (args[i] != "--no-module") {
            usage = true;
        }
    }
    if (usage) {
        std::cerr << "usage: kiln check [plugin|module.wasm] [--target server|proxy]\n";
        return kUsageError;
    }
    const fs::path input = given.value_or(fs::path{"."});
    std::vector<Diagnostic> problems;
    if (fs::is_regular_file(input)) {
        const auto bytes = wire::io::fileIO::readBytes(input.string());
        if (!bytes.has_value()) {
            std::cerr << input.string() << ": cannot read\n";
            return 1;
        }
        problems = verifyModule(*bytes, input.filename().string(), target.value_or("server"));
    } else if (isPluginFolder(input)) {
        problems = verifyPlugin(input, target);
        if (problems.empty() && !fs::is_regular_file(builtModulePath(input, "plugin.wasm")) &&
            !fs::exists(input / "build")) {
            std::cerr << "kiln check: the module is not built yet; run `kiln build` to check it too\n";
        }
    } else {
        std::cerr << input.string() << " is not a plugin folder or a .wasm module\n";
        return kUsageError;
    }
    print(problems);
    if (problems.empty()) {
        std::cout << "ok\n";
    }
    return problems.empty() ? 0 : 1;
}

int runPack(const std::vector<std::string>& args) {
    const std::optional<Parsed> parsed = parse(args, false, true);
    if (!parsed.has_value()) {
        std::cerr << "usage: kiln pack [plugin] [--target server|proxy]\n";
        return kUsageError;
    }
    const std::vector<Diagnostic> problems = verifyPlugin(parsed->plugin, parsed->target);
    if (!problems.empty()) {
        print(problems);
        return 1;
    }
    auto project = Project::open(parsed->plugin);
    if (!project.has_value()) {
        std::cerr << "kiln pack: " << project.error() << '\n';
        return 1;
    }
    const std::string target = parsed->target.value_or(manifestTarget(parsed->plugin));
    const auto output = pack(*project, PackOptions{.packageExtension = targetNamed(target)->package});
    if (!output.has_value()) {
        std::cerr << "kiln pack: " << output.error() << '\n';
        return 1;
    }
    std::cout << "Packed " << output->string() << '\n';
    return 0;
}

int runToolchain(const std::vector<std::string>& args) {
    const std::string action = args.size() > 1 ? args[1] : "list";
    if (action == "list" && args.size() <= 2) {
        std::cout << "cache: " << cacheSettings().root.string() << "\nplatform: " << currentPlatform() << "\n\n";
        for (const std::string_view id : toolchainIds()) {
            const bool available = !pinsFor(id, currentPlatform()).empty();
            std::cout << id << ' ' << toolchainVersion(id) << "  "
                      << (isInstalled(id) ? "installed"
                          : available     ? "download " + formatBytes(downloadSize(id))
                                          : "unavailable on this platform")
                      << '\n';
        }
        return 0;
    }
    if (action == "clean" && args.size() == 2) {
        if (const auto cleaned = cleanCache(); !cleaned.has_value()) {
            std::cerr << "kiln toolchain: " << cleaned.error() << '\n';
            return 1;
        }
        std::cout << "Removed the toolchains and downloads under " << cacheSettings().root.string() << '\n';
        return 0;
    }
    if ((action == "install" || action == "path") && args.size() == 3) {
        const std::vector<std::string_view> ids = toolchainsFor(args[2]);
        if (ids.empty()) {
            std::cerr << "kiln toolchain: nothing to " << action << " for " << args[2] << "; use a language ("
                      << languageList() << ") or a toolchain id\n";
            return 1;
        }
        for (const std::string_view id : ids) {
            if (action == "path" && !isInstalled(id)) {
                std::cerr << "kiln toolchain: " << id << " is not installed\n";
                return 1;
            }
            const auto dir = ensureToolchain(id);
            if (!dir.has_value()) {
                std::cerr << "kiln toolchain: " << dir.error() << '\n';
                return 1;
            }
            std::cout << (action == "path" ? "" : std::string{id} + ": ") << dir->string() << '\n';
        }
        return 0;
    }
    std::cerr << "usage: kiln toolchain list | install <language|id> | path <language|id> | clean\n";
    return kUsageError;
}

} // namespace basalt::kiln
