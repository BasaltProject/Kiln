#pragma once

#include <string_view>
#include <vector>

namespace basalt::kiln {

struct EmbeddedFile {
    std::string_view path;
    std::string_view contents;
};

/// `sdk/<lang>/...`: the Basalt bindings for each language, copied next to the build of a plugin.
extern const std::vector<EmbeddedFile> kSdkFiles;

/// `templates/<lang>/...`: what `kiln new` writes, with `@ID@` and `@NAME@` replaced.
extern const std::vector<EmbeddedFile> kTemplateFiles;

/// The normative ABI header, as shipped in Wire.
extern const std::vector<EmbeddedFile> kAbiFiles;

} // namespace basalt::kiln
