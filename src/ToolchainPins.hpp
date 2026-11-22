#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace basalt::kiln {

enum class PinKind { TarGz, TarXz, Zip, Raw };

/// One pinned download: where it is, the hash it must have, and how it unpacks into the toolchain folder.
struct Pin {
    std::string_view id;
    std::string_view version;
    std::string_view platform;
    std::string_view url;
    std::string_view sha256;
    std::uint64_t size;
    PinKind kind;
    std::string_view into;
    int skip;
    std::string_view only;
};

extern const std::vector<Pin> kPins;

} // namespace basalt::kiln
