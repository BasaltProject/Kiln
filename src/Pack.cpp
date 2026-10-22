#include "Pack.hpp"

#include <basalt/wire/io/FileIO.hpp>
#include <basalt/wire/io/ZipWriter.hpp>

#include <algorithm>
#include <vector>

namespace fs = std::filesystem;

namespace basalt::kiln {

std::expected<fs::path, std::string> pack(const Project& project, const PackOptions& options) {
    fs::path source = project.root;
    std::vector<std::string> files = project.files;
    if (options.compiled) {
        source = project.root / "build" / "compiled";
        if (const std::string failure = buildPackage(project.root, source, options.build); !failure.empty()) {
            return std::unexpected(failure);
        }
        files.clear();
        for (const fs::directory_entry& entry : fs::recursive_directory_iterator(source)) {
            if (entry.is_regular_file()) {
                files.push_back(fs::relative(entry.path(), source).generic_string());
            }
        }
        std::ranges::sort(files);
    }
    wire::io::ZipWriter zip;
    for (const std::string& file : files) {
        const auto bytes = wire::io::fileIO::readBytes((source / file).string());
        if (!bytes.has_value()) {
            return std::unexpected("cannot read " + file);
        }
        zip.add(file, *bytes);
    }
    const fs::path output = project.root / "build" / project.packageName();
    fs::create_directories(output.parent_path());
    if (!wire::io::fileIO::writeBytes(output.string(), zip.finish())) {
        return std::unexpected("cannot write " + output.string());
    }
    return output;
}

} // namespace basalt::kiln
