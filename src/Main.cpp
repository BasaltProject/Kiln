#include "Compiler.hpp"
#include "Plugin.hpp"
#include "Targets.hpp"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr std::string_view kUsage = "usage: kiln compile <file.lua|dir> [-o "
                                    "<out>] [--strip] [--target server|proxy]\n"
                                    "       kiln check <plugin|file.lua|dir> [--target server|proxy]\n";

std::vector<fs::path> luaFilesIn(const fs::path& input) {
    std::vector<fs::path> files;
    if (fs::is_directory(input)) {
        for (const fs::directory_entry& entry : fs::recursive_directory_iterator(input)) {
            if (entry.is_regular_file() && entry.path().extension() == ".lua") {
                files.push_back(entry.path());
            }
        }
        std::ranges::sort(files);
    } else {
        files.push_back(input);
    }
    return files;
}

bool readFile(const fs::path& path, std::string& out) {
    std::ifstream in{path, std::ios::binary};
    if (!in) {
        return false;
    }
    out.assign(std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{});
    return true;
}

} // namespace

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);
    if (args.size() < 2 || (args[0] != "compile" && args[0] != "check")) {
        std::cerr << kUsage;
        return 2;
    }
    const bool check = args[0] == "check";
    basalt::kiln::Options options;
    basalt::kiln::Target target = *basalt::kiln::targetNamed("server");
    fs::path input{args[1]};
    std::optional<fs::path> output;
    bool verify = true;
    for (std::size_t i = 2; i < args.size(); ++i) {
        if (args[i] == "--strip") {
            options.strip = true;
        } else if (args[i] == "--no-verify") {
            verify = false;
        } else if (args[i] == "--target" && i + 1 < args.size()) {
            const auto named = basalt::kiln::targetNamed(args[++i]);
            if (!named.has_value()) {
                std::cerr << "unknown target " << args[i] << '\n';
                return 2;
            }
            target = *named;
        } else if (args[i] == "-o" && i + 1 < args.size()) {
            output = args[++i];
        } else {
            std::cerr << kUsage;
            return 2;
        }
    }
    if (basalt::kiln::isPluginFolder(input)) {
        if (verify) {
            const std::vector<basalt::kiln::Diagnostic> problems = basalt::kiln::verifyPlugin(input, target.name);
            for (const basalt::kiln::Diagnostic& problem : problems) {
                std::cerr << problem.file << (problem.line > 0 ? ":" + std::to_string(problem.line) : "") << ": "
                          << problem.message << '\n';
            }
            if (!problems.empty()) {
                std::cerr << problems.size() << " problem(s)\n";
                return 1;
            }
        }
        if (check) {
            std::cout << "ok\n";
            return 0;
        }
        output = output.value_or(input / "build" / "luac");
        input /= "src";
    }
    const bool directory = fs::is_directory(input);
    int failed = 0;
    for (const fs::path& file : luaFilesIn(input)) {
        std::string source;
        if (!readFile(file, source)) {
            std::cerr << file.string() << ": cannot read\n";
            ++failed;
            continue;
        }
        const fs::path relative = directory ? fs::relative(file, input) : file.filename();
        const auto bytecode = basalt::kiln::compile(source, relative.generic_string(), options);
        if (!bytecode.has_value()) {
            std::cerr << bytecode.error() << '\n';
            ++failed;
            continue;
        }
        if (check) {
            continue;
        }
        fs::path destination = output.value_or(fs::path{file}.replace_extension(target.extension));
        if (directory) {
            destination = output.value_or(input) / relative;
            destination.replace_extension(target.extension);
        }
        if (destination.has_parent_path()) {
            fs::create_directories(destination.parent_path());
        }
        std::ofstream out{destination, std::ios::binary};
        out.write(bytecode->data(), static_cast<std::streamsize>(bytecode->size()));
        if (!out) {
            std::cerr << destination.string() << ": cannot write\n";
            ++failed;
        }
    }
    return failed == 0 ? 0 : 1;
}
