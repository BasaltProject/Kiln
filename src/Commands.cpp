#include "Commands.hpp"

#include "Pack.hpp"
#include "Plugin.hpp"
#include "Project.hpp"
#include "Scaffold.hpp"
#include "Targets.hpp"
#include "Testing.hpp"

#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

namespace basalt::kiln {
namespace {

constexpr int kUsageError = 2;

struct Parsed {
    fs::path plugin{"."};
    std::string_view target = "server";
    PackOptions pack;
};

/// Opens the plugin and verifies it for the target; prints every problem and returns nothing if there is one.
std::optional<Project> openVerified(const Parsed& parsed, const char* command) {
    const std::vector<Diagnostic> problems = verifyPlugin(parsed.plugin, parsed.target);
    for (const Diagnostic& problem : problems) {
        std::cerr << problem.file << (problem.line > 0 ? ":" + std::to_string(problem.line) : "") << ": "
                  << problem.message << '\n';
    }
    if (!problems.empty()) {
        std::cerr << problems.size() << " problem(s)\n";
        return std::nullopt;
    }
    auto project = Project::open(parsed.plugin);
    if (!project.has_value()) {
        std::cerr << "kiln " << command << ": " << project.error() << '\n';
        return std::nullopt;
    }
    return *std::move(project);
}

std::optional<Parsed> parse(const std::vector<std::string>& args, bool packing) {
    Parsed parsed;
    std::size_t i = 1;
    if (args.size() > 1 && !args[1].starts_with("-")) {
        parsed.plugin = args[i++];
    }
    for (; i < args.size(); ++i) {
        if (args[i] == "--target" && i + 1 < args.size()) {
            const auto named = targetNamed(args[++i]);
            if (!named.has_value()) {
                std::cerr << "unknown target " << args[i] << '\n';
                return std::nullopt;
            }
            parsed.target = named->name;
        } else if (packing && args[i] == "--compiled") {
            parsed.pack.compiled = true;
        } else if (packing && args[i] == "--strip") {
            parsed.pack.build.strip = true;
        } else if (packing && args[i] == "--sign" && i + 1 < args.size()) {
            parsed.pack.build.signingKey = args[++i];
        } else {
            return std::nullopt;
        }
    }
    return parsed;
}

} // namespace

int runNew(const std::vector<std::string>& args) {
    if (args.size() < 2 || args.size() > 3) {
        std::cerr << "usage: kiln new <id> [name]\n";
        return kUsageError;
    }
    if (const std::string failure = scaffold(".", args[1], args.size() > 2 ? args[2] : args[1]); !failure.empty()) {
        std::cerr << "kiln new: " << failure << '\n';
        return 1;
    }
    std::cout << "Created " << args[1] << "/\n";
    return 0;
}

int runPack(const std::vector<std::string>& args) {
    const std::optional<Parsed> parsed = parse(args, true);
    if (!parsed.has_value()) {
        std::cerr << "usage: kiln pack [plugin] [--compiled] [--sign <key>] [--strip] [--target server|proxy]\n";
        return kUsageError;
    }
    const std::optional<Project> project = openVerified(*parsed, "pack");
    if (!project.has_value()) {
        return 1;
    }
    const auto output = pack(*project, parsed->pack);
    if (!output.has_value()) {
        std::cerr << "kiln pack: " << output.error() << '\n';
        return 1;
    }
    std::cout << "Packed " << output->string() << '\n';
    return 0;
}

int runTest(const std::vector<std::string>& args) {
    const std::optional<Parsed> parsed = parse(args, false);
    if (!parsed.has_value()) {
        std::cerr << "usage: kiln test [plugin] [--target server|proxy]\n";
        return kUsageError;
    }
    const std::optional<Project> project = openVerified(*parsed, "test");
    return project.has_value() && runTests(*project) ? 0 : 1;
}

} // namespace basalt::kiln
