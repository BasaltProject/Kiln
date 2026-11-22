#include "Scaffold.hpp"

#include "Embedded.hpp"
#include "Plugin.hpp"

#include <basalt/wire/io/FileIO.hpp>

namespace fs = std::filesystem;

namespace basalt::kiln {
namespace {

std::string replaced(std::string_view text, std::string_view id, std::string_view name, std::string_view language) {
    std::string out{text};
    for (const auto& [key, value] :
         {std::pair{std::string_view{"@ID@"}, id}, std::pair{std::string_view{"@NAME@"}, name},
          std::pair{std::string_view{"@LANG@"}, language}}) {
        for (std::size_t at = out.find(key); at != std::string::npos; at = out.find(key, at + value.size())) {
            out.replace(at, key.size(), value);
        }
    }
    return out;
}

} // namespace

std::string scaffold(const fs::path& root, std::string_view id, std::string_view name, Language language,
                     std::string_view target) {
    if (!validPluginId(id)) {
        return "the id must be lower case letters, digits or '_', and start with a letter";
    }
    const std::string languageName{infoOf(language).name};
    const std::string prefix = target == "proxy" ? "proxy-" : "";
    const std::string folderName = prefix + languageName;
    std::vector<const EmbeddedFile*> files;
    bool hasLanguage = false;
    for (const EmbeddedFile& file : kTemplateFiles) {
        const bool own = file.path.starts_with(folderName + "/");
        hasLanguage = hasLanguage || own;
        if (own || file.path.starts_with(prefix + "common/")) {
            files.push_back(&file);
        }
    }
    if (!hasLanguage) {
        return "there is no " + std::string{target} + " template for " + std::string{infoOf(language).title} + " yet";
    }
    const fs::path folder = root / std::string{id};
    if (fs::exists(folder)) {
        return folder.string() + " already exists";
    }
    for (const EmbeddedFile* file : files) {
        std::string relative{file->path.substr(file->path.find('/') + 1)};
        if (relative == "gitignore") {
            relative = ".gitignore";
        }
        fs::create_directories((folder / relative).parent_path());
        if (!wire::io::fileIO::writeText((folder / relative).string(),
                                         replaced(file->contents, id, name, languageName))) {
            return "could not write " + (folder / relative).string();
        }
    }
    return {};
}

} // namespace basalt::kiln
