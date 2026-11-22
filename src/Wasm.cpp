#include "Wasm.hpp"

#include <algorithm>
#include <array>
#include <limits>

namespace basalt::kiln::wasm {
namespace {

class Reader {
public:
    explicit Reader(std::span<const std::uint8_t> bytes) : bytes_(bytes) {}

    [[nodiscard]] bool atEnd() const { return position_ >= bytes_.size(); }

    [[nodiscard]] std::size_t position() const { return position_; }

    [[nodiscard]] bool failed() const { return failed_; }

    std::uint8_t byte() {
        if (position_ >= bytes_.size()) {
            failed_ = true;
            return 0;
        }
        return bytes_[position_++];
    }

    std::uint64_t unsignedLeb() {
        std::uint64_t value = 0;
        for (unsigned shift = 0; shift < 70; shift += 7) {
            const std::uint8_t next = byte();
            if (failed_) {
                return 0;
            }
            if (shift < 64) {
                value |= static_cast<std::uint64_t>(next & 0x7F) << shift;
            }
            if ((next & 0x80) == 0) {
                return value;
            }
        }
        failed_ = true;
        return 0;
    }

    std::uint32_t u32() {
        const std::uint64_t value = unsignedLeb();
        if (value > std::numeric_limits<std::uint32_t>::max()) {
            failed_ = true;
            return 0;
        }
        return static_cast<std::uint32_t>(value);
    }

    void signedLeb() {
        for (unsigned count = 0; count < 10; ++count) {
            if ((byte() & 0x80) == 0 || failed_) {
                return;
            }
        }
        failed_ = true;
    }

    std::string name() {
        const std::uint32_t length = u32();
        if (failed_ || length > bytes_.size() - position_) {
            failed_ = true;
            return {};
        }
        std::string text{reinterpret_cast<const char*>(bytes_.data() + position_), length};
        position_ += length;
        return text;
    }

    void skip(std::size_t count) {
        if (count > bytes_.size() - position_) {
            failed_ = true;
            return;
        }
        position_ += count;
    }

    [[nodiscard]] Reader slice(std::size_t length) {
        if (length > bytes_.size() - position_) {
            failed_ = true;
            return Reader{{}};
        }
        Reader inner{bytes_.subspan(position_, length)};
        position_ += length;
        return inner;
    }

private:
    std::span<const std::uint8_t> bytes_;
    std::size_t position_ = 0;
    bool failed_ = false;
};

constexpr std::uint8_t kRefNullable = 0x63;
constexpr std::uint8_t kRefNonNull = 0x64;

/// Reads a value type; a reference type with a heap type index is two parts, anything else a single byte.
std::uint8_t valueType(Reader& in, bool& gc) {
    const std::uint8_t first = in.byte();
    if (first == kRefNullable || first == kRefNonNull) {
        in.signedLeb();
        gc = true;
    } else if (first != 0x7F && first != 0x7E && first != 0x7D && first != 0x7C && first != 0x7B && first != 0x70 &&
               first != 0x6F) {
        gc = true;
    }
    return first;
}

void limits(Reader& in, std::uint64_t& minimum) {
    const std::uint8_t flags = in.byte();
    minimum = in.unsignedLeb();
    if ((flags & 0x01) != 0) {
        in.unsignedLeb();
    }
}

void parseSubType(Reader& in, Module& module, std::uint8_t tag) {
    if (tag == 0x50 || tag == 0x4F) {
        const std::uint32_t supers = in.u32();
        for (std::uint32_t i = 0; i < supers && !in.failed(); ++i) {
            in.u32();
        }
        module.usesGc = true;
        tag = in.byte();
    }
    if (tag == 0x60) {
        FuncType type;
        const std::uint32_t parameters = in.u32();
        for (std::uint32_t i = 0; i < parameters && !in.failed(); ++i) {
            type.parameters.push_back(valueType(in, module.usesGc));
        }
        const std::uint32_t results = in.u32();
        for (std::uint32_t i = 0; i < results && !in.failed(); ++i) {
            type.results.push_back(valueType(in, module.usesGc));
        }
        module.types.emplace_back(std::move(type));
    } else if (tag == 0x5F) {
        module.usesGc = true;
        const std::uint32_t fields = in.u32();
        for (std::uint32_t i = 0; i < fields && !in.failed(); ++i) {
            valueType(in, module.usesGc);
            in.byte();
        }
        module.types.emplace_back(std::nullopt);
    } else if (tag == 0x5E) {
        module.usesGc = true;
        valueType(in, module.usesGc);
        in.byte();
        module.types.emplace_back(std::nullopt);
    } else {
        in.skip(std::numeric_limits<std::size_t>::max());
    }
}

void parseTypes(Reader& in, Module& module) {
    const std::uint32_t count = in.u32();
    for (std::uint32_t i = 0; i < count && !in.failed(); ++i) {
        const std::uint8_t tag = in.byte();
        if (tag == 0x4E) {
            module.usesGc = true;
            const std::uint32_t members = in.u32();
            for (std::uint32_t m = 0; m < members && !in.failed(); ++m) {
                parseSubType(in, module, in.byte());
            }
        } else {
            parseSubType(in, module, tag);
        }
    }
}

void parseImports(Reader& in, Module& module) {
    const std::uint32_t count = in.u32();
    for (std::uint32_t i = 0; i < count && !in.failed(); ++i) {
        Import entry;
        entry.module = in.name();
        entry.name = in.name();
        entry.kind = in.byte();
        std::uint64_t ignored = 0;
        switch (entry.kind) {
        case kFunction:
            entry.typeIndex = in.u32();
            ++module.importedFunctions;
            break;
        case kTable:
            valueType(in, module.usesGc);
            limits(in, ignored);
            break;
        case kMemory:
            limits(in, ignored);
            ++module.importedMemories;
            break;
        case kGlobal:
            valueType(in, module.usesGc);
            in.byte();
            break;
        case kTag:
            in.byte();
            entry.typeIndex = in.u32();
            break;
        default:
            in.skip(std::numeric_limits<std::size_t>::max());
            break;
        }
        module.imports.push_back(std::move(entry));
    }
}

void parseExports(Reader& in, Module& module) {
    const std::uint32_t count = in.u32();
    for (std::uint32_t i = 0; i < count && !in.failed(); ++i) {
        Export entry;
        entry.name = in.name();
        entry.kind = in.byte();
        entry.index = in.u32();
        module.exports.push_back(std::move(entry));
    }
}

} // namespace

const FuncType* Module::signatureOf(const Import& function) const {
    if (function.kind != kFunction || function.typeIndex >= types.size() || !types[function.typeIndex].has_value()) {
        return nullptr;
    }
    return &*types[function.typeIndex];
}

const FuncType* Module::signatureOf(const Export& function) const {
    if (function.kind != kFunction || function.index < importedFunctions) {
        return nullptr;
    }
    const std::uint32_t local = function.index - importedFunctions;
    if (local >= definedFunctionTypes.size() || definedFunctionTypes[local] >= types.size() ||
        !types[definedFunctionTypes[local]].has_value()) {
        return nullptr;
    }
    return &*types[definedFunctionTypes[local]];
}

std::expected<Module, std::string> parse(std::span<const std::uint8_t> bytes) {
    constexpr std::array<std::uint8_t, 8> kHeader{0x00, 0x61, 0x73, 0x6D, 0x01, 0x00, 0x00, 0x00};
    if (bytes.size() < kHeader.size() || !std::equal(kHeader.begin(), kHeader.end(), bytes.begin())) {
        return std::unexpected("not a WebAssembly binary module (bad magic or version)");
    }
    Module module;
    module.size = bytes.size();
    Reader in{bytes.subspan(kHeader.size())};
    while (!in.atEnd()) {
        const std::uint8_t id = in.byte();
        const std::uint32_t length = in.u32();
        Reader section = in.slice(length);
        if (in.failed()) {
            return std::unexpected("truncated section " + std::to_string(id));
        }
        switch (id) {
        case 1:
            parseTypes(section, module);
            break;
        case 2:
            parseImports(section, module);
            break;
        case 3: {
            const std::uint32_t count = section.u32();
            for (std::uint32_t i = 0; i < count && !section.failed(); ++i) {
                module.definedFunctionTypes.push_back(section.u32());
            }
            break;
        }
        case 5: {
            const std::uint32_t count = section.u32();
            for (std::uint32_t i = 0; i < count && !section.failed(); ++i) {
                std::uint64_t minimum = 0;
                limits(section, minimum);
                module.memoryMinPages = std::max(module.memoryMinPages, minimum);
                ++module.definedMemories;
            }
            break;
        }
        case 7:
            parseExports(section, module);
            break;
        default:
            break;
        }
        if (section.failed()) {
            return std::unexpected("malformed section " + std::to_string(id));
        }
    }
    return module;
}

} // namespace basalt::kiln::wasm
