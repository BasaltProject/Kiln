#include "Pack.hpp"

#include <basalt/wire/io/FileIO.hpp>
#include <basalt/wire/io/ZipWriter.hpp>

namespace fs = std::filesystem;

namespace basalt::kiln {

std::expected<fs::path, std::string> pack(const Project& project, const PackOptions& options) {
    wire::io::ZipWriter zip;
    for (const ProjectFile& file : project.files) {
        const auto bytes = wire::io::fileIO::readBytes(file.source.string());
        if (!bytes.has_value()) {
            return std::unexpected("cannot read " + file.source.string());
        }
        zip.add(file.name, *bytes);
    }
    const fs::path output = project.root / "build" / project.packageName(options.packageExtension);
    fs::create_directories(output.parent_path());
    if (!wire::io::fileIO::writeBytes(output.string(), zip.finish())) {
        return std::unexpected("cannot write " + output.string());
    }
    return output;
}

} // namespace basalt::kiln
