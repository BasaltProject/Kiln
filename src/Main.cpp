#include "BuildInfo.hpp"
#include "Commands.hpp"
#include "Toolchain.hpp"
#include "Update.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr std::string_view kUsage = "usage: kiln new <id> [name] [--lang <language>] [--target server|proxy]\n"
                                    "       kiln build [plugin] [--lang <language>] [--target server|proxy] [-v]\n"
                                    "       kiln check [plugin|module.wasm] [--target server|proxy]\n"
                                    "       kiln pack [plugin] [--target server|proxy]\n"
                                    "       kiln toolchain list | install <language|id> | path <language|id> | clean\n"
                                    "       kiln update [--check | --auto on|off|status]\n"
                                    "       kiln --version\n"
                                    "options: --cache-dir <dir>  --offline  --no-download\n";

} // namespace

int main(int argc, char** argv) {
    std::vector<std::string> args;
    basalt::kiln::CacheSettings& cache = basalt::kiln::cacheSettings();
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--cache-dir" && i + 1 < argc) {
            cache.root = argv[++i];
        } else if (argument == "--offline") {
            cache.offline = true;
        } else if (argument == "--no-download") {
            cache.noDownload = true;
        } else {
            args.push_back(argument);
        }
    }
    if (args.size() == 1 && (args[0] == "--version" || args[0] == "-V")) {
        std::cout << "Kiln " << basalt::kiln::buildInfo::kVersion << '\n';
        return 0;
    }
    if (cache.root.empty()) {
        cache.root = basalt::kiln::defaultCacheRoot();
    }
    if (!args.empty()) {
        if (args[0] == "__apply-update") {
            return basalt::kiln::applyWindowsUpdate(args);
        }
        if (args[0] == "update") {
            return basalt::kiln::runUpdate(args, basalt::kiln::buildInfo::kVersion);
        }
        int result = 2;
        if (args[0] == "new") {
            result = basalt::kiln::runNew(args);
        } else if (args[0] == "build") {
            result = basalt::kiln::runBuild(args);
        } else if (args[0] == "check") {
            result = basalt::kiln::runCheck(args);
        } else if (args[0] == "pack") {
            result = basalt::kiln::runPack(args);
        } else if (args[0] == "toolchain") {
            result = basalt::kiln::runToolchain(args);
        } else {
            std::cerr << kUsage;
            return 2;
        }
        if (result == 0) {
            basalt::kiln::autoUpdate(basalt::kiln::buildInfo::kVersion);
        }
        return result;
    }
    std::cerr << kUsage;
    return 2;
}
