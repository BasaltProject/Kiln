#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "Wasm.hpp"

#include "AbiTable.hpp"
#include "WasmCheck.hpp"

#include <doctest/doctest.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace {

using Bytes = std::vector<std::uint8_t>;

void section(Bytes& out, std::uint8_t id, const Bytes& body) {
    out.push_back(id);
    out.push_back(static_cast<std::uint8_t>(body.size()));
    out.insert(out.end(), body.begin(), body.end());
}

void name(Bytes& out, const std::string& text) {
    out.push_back(static_cast<std::uint8_t>(text.size()));
    out.insert(out.end(), text.begin(), text.end());
}

// log(i32, i32, i32), basalt_abi_version() -> i32, basalt_init(), basalt_callback(i32, i32), one memory.
Bytes pluginModule(std::uint8_t logParameter) {
    Bytes out{0x00, 0x61, 0x73, 0x6D, 0x01, 0x00, 0x00, 0x00};
    section(out, 1, {4, 0x60, 3, 0x7F, 0x7F, logParameter, 0, 0x60, 0, 1, 0x7F, 0x60, 0, 0, 0x60, 2, 0x7F, 0x7F, 0});
    Bytes imports{1};
    name(imports, "basalt");
    name(imports, "log");
    imports.insert(imports.end(), {0, 0});
    section(out, 2, imports);
    section(out, 3, {3, 1, 2, 3});
    section(out, 5, {1, 0, 1});
    Bytes exports{4};
    name(exports, "memory");
    exports.insert(exports.end(), {2, 0});
    name(exports, "basalt_abi_version");
    exports.insert(exports.end(), {0, 1});
    name(exports, "basalt_init");
    exports.insert(exports.end(), {0, 2});
    name(exports, "basalt_callback");
    exports.insert(exports.end(), {0, 3});
    section(out, 7, exports);
    Bytes code{3, 4, 0, 0x41, 1, 0x0B, 2, 0, 0x0B, 2, 0, 0x0B};
    section(out, 10, code);
    return out;
}

} // namespace

TEST_CASE("the parser reads imports, exports and signatures back") {
    const Bytes module = pluginModule(0x7F);
    const auto parsed = basalt::kiln::wasm::parse(module);
    REQUIRE(parsed.has_value());
    REQUIRE(parsed->imports.size() == 1);
    CHECK(parsed->imports[0].module == "basalt");
    CHECK(parsed->imports[0].name == "log");
    REQUIRE(parsed->signatureOf(parsed->imports[0]) != nullptr);
    CHECK(parsed->signatureOf(parsed->imports[0])->parameters.size() == 3);
    CHECK(parsed->exports.size() == 4);
    CHECK(parsed->definedMemories == 1);
}

TEST_CASE("the checker accepts a module that follows the ABI and rejects a wrong import signature") {
    CHECK(basalt::kiln::verifyModule(pluginModule(0x7F), "plugin.wasm").empty());
    CHECK_FALSE(basalt::kiln::verifyModule(pluginModule(0x7E), "plugin.wasm").empty());
}

TEST_CASE("the ABI table is derived from the header") {
    using basalt::kiln::abi::kImports;
    const auto log = std::ranges::find(kImports, "log", &basalt::kiln::abi::ImportSpec::name);
    REQUIRE(log != kImports.end());
    CHECK(log->parameters == "iii");
    CHECK(log->result == 'v');
    const auto config = std::ranges::find(kImports, "config_i64", &basalt::kiln::abi::ImportSpec::name);
    REQUIRE(config != kImports.end());
    CHECK(config->parameters == "iil");
    CHECK(config->result == 'l');
}
