#pragma once

#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace basalt::kiln::wasm {

inline constexpr std::uint8_t kFunction = 0;
inline constexpr std::uint8_t kTable = 1;
inline constexpr std::uint8_t kMemory = 2;
inline constexpr std::uint8_t kGlobal = 3;
inline constexpr std::uint8_t kTag = 4;

struct FuncType {
    std::vector<std::uint8_t> parameters;
    std::vector<std::uint8_t> results;
};

struct Import {
    std::string module;
    std::string name;
    std::uint8_t kind = kFunction;
    std::uint32_t typeIndex = 0;
};

struct Export {
    std::string name;
    std::uint8_t kind = kFunction;
    std::uint32_t index = 0;
};

/// What the plugin checker needs from a binary module: its imports, exports, memory and function signatures.
struct Module {
    std::vector<Import> imports;
    std::vector<Export> exports;
    /// One entry per type index; a struct or array type leaves its entry empty.
    std::vector<std::optional<FuncType>> types;
    std::vector<std::uint32_t> definedFunctionTypes;
    std::uint32_t importedFunctions = 0;
    std::uint32_t definedMemories = 0;
    std::uint32_t importedMemories = 0;
    std::uint64_t memoryMinPages = 0;
    bool usesGc = false;
    std::size_t size = 0;

    [[nodiscard]] const FuncType* signatureOf(const Import& function) const;

    [[nodiscard]] const FuncType* signatureOf(const Export& function) const;
};

[[nodiscard]] std::expected<Module, std::string> parse(std::span<const std::uint8_t> bytes);

} // namespace basalt::kiln::wasm
