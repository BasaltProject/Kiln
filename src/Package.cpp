#include "Package.hpp"

#include "Compiler.hpp"

#include <basalt/wire/plugins/PluginPackage.hpp>
#include <basalt/wire/plugins/PluginSignature.hpp>

#include <algorithm>
#include <fstream>
#include <iterator>
#include <random>
#include <vector>

namespace fs = std::filesystem;
namespace signature = basalt::wire::plugins::signature;

namespace basalt::kiln {
namespace {

bool readFile(const fs::path& path, std::string& out) {
    std::ifstream in{path, std::ios::binary};
    if (!in) {
        return false;
    }
    out.assign(std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{});
    return true;
}

bool writeFile(const fs::path& path, std::string_view bytes) {
    fs::create_directories(path.parent_path());
    std::ofstream out{path, std::ios::binary};
    out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    return static_cast<bool>(out);
}

std::string trimmed(std::string text) {
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r' || text.back() == ' ')) {
        text.pop_back();
    }
    return text;
}

} // namespace

std::string generateKey(const fs::path& path) {
    signature::Seed seed{};
    std::random_device device;
    for (std::uint8_t& byte : seed) {
        byte = static_cast<std::uint8_t>(device());
    }
    fs::path secret = path;
    secret += ".key";
    fs::path pub = path;
    pub += ".pub";
    if (fs::exists(secret)) {
        return secret.string() + " already exists; refusing to replace a key";
    }
    if (!writeFile(secret, signature::toHex(seed) + "\n") ||
        !writeFile(pub, signature::toHex(signature::publicKeyOf(seed)) + "\n")) {
        return "cannot write the key files next to " + path.string();
    }
    fs::permissions(secret, fs::perms::owner_read | fs::perms::owner_write, fs::perm_options::replace);
    return {};
}

std::string buildPackage(const fs::path& plugin, const fs::path& output, const BuildOptions& options) {
    std::optional<signature::Seed> seed;
    if (!options.signingKey.empty()) {
        std::string text;
        if (!readFile(options.signingKey, text)) {
            return "cannot read the signing key " + options.signingKey.string();
        }
        const auto bytes = signature::fromHex(trimmed(text), 32);
        if (!bytes.has_value()) {
            return options.signingKey.string() + " is not a key made by `kiln keygen`";
        }
        seed = signature::Seed{};
        std::ranges::copy(*bytes, seed->begin());
    }
    if (fs::exists(output)) {
        fs::remove_all(output);
    }
    fs::create_directories(output);
    fs::copy_file(plugin / "plugin.toml", output / "plugin.toml");
    if (fs::is_directory(plugin / "resources")) {
        fs::copy(plugin / "resources", output / "resources", fs::copy_options::recursive);
    }
    const fs::path sources = plugin / "src";
    if (fs::is_directory(sources)) {
        for (const fs::directory_entry& entry : fs::recursive_directory_iterator(sources)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            const fs::path relative = fs::relative(entry.path(), plugin);
            std::string source;
            if (!readFile(entry.path(), source)) {
                return entry.path().string() + ": cannot read";
            }
            if (entry.path().extension() != ".lua") {
                if (!writeFile(output / relative, source)) {
                    return (output / relative).string() + ": cannot write";
                }
                continue;
            }
            const auto bytecode = compile(source, relative.generic_string(), Options{.strip = options.strip});
            if (!bytecode.has_value()) {
                return bytecode.error();
            }
            fs::path destination = output / relative;
            destination.replace_extension(".luac");
            if (!writeFile(destination, *bytecode)) {
                return destination.string() + ": cannot write";
            }
        }
    }
    if (seed.has_value()) {
        const basalt::wire::plugins::FolderPackage built{output.string()};
        const signature::Signature value = signature::sign(*seed, signature::payload(built));
        if (!writeFile(output / std::string{signature::kFile},
                       signature::render(signature::publicKeyOf(*seed), value))) {
            return "cannot write " + std::string{signature::kFile};
        }
    }
    return {};
}

} // namespace basalt::kiln
