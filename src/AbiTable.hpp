#pragma once

#include <string_view>
#include <vector>

namespace basalt::kiln::abi {

/// One function the plugin may import from an ABI module; the signature has one letter per parameter (i, l, f,
/// d). `shared` marks a function of the module `basalt` that the proxy profile offers too.
struct ImportSpec {
    std::string_view name;
    std::string_view parameters;
    char result;
    bool shared;
};

extern const std::vector<ImportSpec> kImports;

/// The functions of the module `basalt_proxy`, named without the `basalt_proxy_` prefix.
extern const std::vector<ImportSpec> kProxyImports;

extern const int kVersion;

} // namespace basalt::kiln::abi
