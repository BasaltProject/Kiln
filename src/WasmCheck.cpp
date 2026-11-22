#include "WasmCheck.hpp"

#include "AbiTable.hpp"
#include "Wasm.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <map>
#include <set>

namespace fs = std::filesystem;

namespace basalt::kiln {
namespace {

constexpr std::size_t kMaxModuleBytes = 16UL * 1024UL * 1024UL;
constexpr std::uint64_t kMaxMemoryPages = 4096;
constexpr std::size_t kMaxImports = 2048;

constexpr std::array<std::string_view, 8> kWasiSubset{
    "fd_write",          "clock_time_get", "random_get",     "environ_get",
    "environ_sizes_get", "args_get",       "args_sizes_get", "proc_exit",
};

struct RequiredExport {
    std::string_view name;
    std::string_view parameters;
    char result;
};

constexpr std::array<RequiredExport, 3> kRequiredExports{{
    {"basalt_abi_version", "", 'i'},
    {"basalt_init", "", 'v'},
    {"basalt_callback", "ii", 'v'},
}};

char letterOf(std::uint8_t type) {
    switch (type) {
    case 0x7F:
        return 'i';
    case 0x7E:
        return 'l';
    case 0x7D:
        return 'f';
    case 0x7C:
        return 'd';
    default:
        return '?';
    }
}

std::string signatureText(std::string_view parameters, char result) {
    std::string text = "(";
    for (std::size_t i = 0; i < parameters.size(); ++i) {
        text += i == 0 ? "" : ", ";
        text += parameters[i] == 'i' ? "i32" : parameters[i] == 'l' ? "i64" : parameters[i] == 'f' ? "f32" : "f64";
    }
    text += ") -> ";
    text += result == 'v' ? "()" : result == 'i' ? "i32" : result == 'l' ? "i64" : result == 'f' ? "f32" : "f64";
    return text;
}

bool matches(const wasm::FuncType& type, std::string_view parameters, char result) {
    std::string have;
    for (const std::uint8_t parameter : type.parameters) {
        have += letterOf(parameter);
    }
    const bool resultMatches =
        result == 'v' ? type.results.empty() : type.results.size() == 1 && letterOf(type.results.front()) == result;
    return have == parameters && resultMatches;
}

std::string describe(const wasm::FuncType& type) {
    std::string parameters;
    for (const std::uint8_t parameter : type.parameters) {
        parameters += letterOf(parameter);
    }
    const char result = type.results.empty() ? 'v' : letterOf(type.results.front());
    return signatureText(parameters, result);
}

} // namespace

bool isWasmMain(const std::string& main) { return main.size() > 5 && main.ends_with(".wasm"); }

fs::path builtModulePath(const fs::path& root, const std::string& main) {
    if (fs::is_regular_file(root / main)) {
        return root / main;
    }
    return root / "build" / fs::path{main}.filename();
}

std::vector<Diagnostic> verifyModule(std::span<const std::uint8_t> bytes, const std::string& file,
                                     std::string_view target) {
    const bool proxy = target == "proxy";
    std::vector<Diagnostic> found;
    const auto problem = [&](std::string message) { found.push_back({file, 0, std::move(message)}); };
    if (bytes.size() > kMaxModuleBytes) {
        problem("the module is " + std::to_string(bytes.size() / (1024 * 1024)) + " MiB, the limit is 16 MiB");
        return found;
    }
    const std::expected<wasm::Module, std::string> parsed = wasm::parse(bytes);
    if (!parsed.has_value()) {
        problem(parsed.error());
        return found;
    }
    const wasm::Module& module = *parsed;
    if (module.imports.size() > kMaxImports) {
        problem("the module has " + std::to_string(module.imports.size()) + " imports, the limit is 2048");
    }

    std::map<std::string_view, const abi::ImportSpec*> known;
    for (const abi::ImportSpec& spec : abi::kImports) {
        if (!proxy || spec.shared) {
            known.emplace(spec.name, &spec);
        }
    }
    std::map<std::string_view, const abi::ImportSpec*> proxyOnly;
    for (const abi::ImportSpec& spec : abi::kProxyImports) {
        proxyOnly.emplace(spec.name, &spec);
    }
    const std::set<std::string_view> wasi(kWasiSubset.begin(), kWasiSubset.end());
    for (const wasm::Import& entry : module.imports) {
        const std::string qualified = entry.module + "." + entry.name;
        if (entry.module == "basalt" || (proxy && entry.module == "basalt_proxy")) {
            const auto& table = entry.module == "basalt" ? known : proxyOnly;
            const auto spec = table.find(entry.name);
            if (spec == table.end()) {
                problem("import " + qualified + " is not part of the " + std::string{target} + " profile of ABI " +
                        std::to_string(abi::kVersion));
            } else if (entry.kind != wasm::kFunction) {
                problem("import " + qualified + " must be a function");
            } else if (const wasm::FuncType* type = module.signatureOf(entry);
                       type != nullptr && !matches(*type, spec->second->parameters, spec->second->result)) {
                problem("import " + qualified + " has type " + describe(*type) + ", the ABI says " +
                        signatureText(spec->second->parameters, spec->second->result));
            }
        } else if (entry.module == "wasi_snapshot_preview1") {
            if (entry.kind != wasm::kFunction || !wasi.contains(entry.name)) {
                problem("import " + qualified + " is outside the WASI subset the host provides");
            }
        } else {
            problem("import " + qualified + " is not available: plugins import only from `basalt`" +
                    (proxy ? " and `basalt_proxy`" : "") +
                    (entry.module == "env" ? " (build for wasm32-unknown-unknown or wasm32-freestanding)" : ""));
        }
    }

    if (module.importedMemories > 0) {
        problem("the module imports its memory; it must define and export its own");
    }
    const auto memory = std::ranges::find_if(
        module.exports, [](const wasm::Export& e) { return e.name == "memory" && e.kind == wasm::kMemory; });
    if (memory == module.exports.end()) {
        problem("the module does not export `memory`");
    }
    if (module.memoryMinPages > kMaxMemoryPages) {
        problem("the module asks for " + std::to_string(module.memoryMinPages) +
                " initial memory pages, the limit is " + std::to_string(kMaxMemoryPages));
    }
    if (const auto init =
            std::ranges::find_if(module.exports, [](const wasm::Export& e) { return e.name == "_initialize"; });
        init != module.exports.end()) {
        const wasm::FuncType* type = init->kind == wasm::kFunction ? module.signatureOf(*init) : nullptr;
        if (type == nullptr || !matches(*type, "", 'v')) {
            problem("export `_initialize` must be a function of type () -> ()");
        }
    }
    for (const RequiredExport& required : kRequiredExports) {
        const auto entry =
            std::ranges::find_if(module.exports, [&](const wasm::Export& e) { return e.name == required.name; });
        if (entry == module.exports.end() || entry->kind != wasm::kFunction) {
            problem("the module does not export the function `" + std::string{required.name} + "`");
        } else if (const wasm::FuncType* type = module.signatureOf(*entry);
                   type != nullptr && !matches(*type, required.parameters, required.result)) {
            problem("export `" + std::string{required.name} + "` has type " + describe(*type) + ", the ABI says " +
                    signatureText(required.parameters, required.result));
        }
    }
    return found;
}

} // namespace basalt::kiln
