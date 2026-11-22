#include "Archive.hpp"

#include "ByteSource.hpp"

#include <basalt/wire/io/FileIO.hpp>
#include <basalt/wire/io/ZipReader.hpp>

#include <algorithm>
#include <array>
#include <charconv>
#include <cstring>
#include <fstream>
#include <vector>
#include <zlib.h>

namespace fs = std::filesystem;

namespace basalt::kiln {
namespace {

constexpr std::size_t kBlock = 512;
constexpr std::uint64_t kMaxExtractedBytes = 8ULL * 1024 * 1024 * 1024;
constexpr std::size_t kMaxEntries = 400000;
constexpr std::size_t kMaxMetadataBytes = 1U << 20;

class FileSource final : public ByteSource {
public:
    explicit FileSource(const fs::path& path) : in_(path, std::ios::binary) {
        if (!in_) {
            error = "cannot read " + path.string();
        }
    }

    std::size_t read(std::span<std::uint8_t> out) override {
        in_.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(out.size()));
        return static_cast<std::size_t>(in_.gcount());
    }

private:
    std::ifstream in_;
};

class GzipSource final : public ByteSource {
public:
    explicit GzipSource(const fs::path& path) : file_(path) {
        if (inflateInit2(&stream_, 15 + 32) != Z_OK) {
            error = "cannot start the gzip decoder";
        }
        error = file_.error.empty() ? error : file_.error;
    }

    GzipSource(const GzipSource&) = delete;
    GzipSource& operator=(const GzipSource&) = delete;

    ~GzipSource() override { inflateEnd(&stream_); }

    std::size_t read(std::span<std::uint8_t> out) override {
        if (!error.empty() || ended_ || out.empty()) {
            return 0;
        }
        stream_.next_out = out.data();
        stream_.avail_out = static_cast<uInt>(out.size());
        while (stream_.avail_out > 0 && !ended_) {
            if (stream_.avail_in == 0) {
                const std::size_t count = file_.read(buffer_);
                if (count == 0) {
                    error = "corrupt gzip data: unexpected end of the file";
                    break;
                }
                stream_.next_in = buffer_.data();
                stream_.avail_in = static_cast<uInt>(count);
            }
            const int code = inflate(&stream_, Z_NO_FLUSH);
            if (code == Z_STREAM_END) {
                ended_ = true;
            } else if (code != Z_OK) {
                error = "corrupt gzip data";
                break;
            }
        }
        return out.size() - stream_.avail_out;
    }

private:
    FileSource file_;
    z_stream stream_{};
    std::array<std::uint8_t, 65536> buffer_{};
    bool ended_ = false;
};

bool isWithin(const fs::path& root, const fs::path& candidate) {
    const fs::path relative = candidate.lexically_relative(root);
    return !relative.empty() && *relative.begin() != "..";
}

std::vector<std::string_view> components(std::string_view path) {
    std::vector<std::string_view> parts;
    std::size_t start = 0;
    while (start <= path.size()) {
        const std::size_t slash = std::min(path.find('/', start), path.size());
        if (slash > start) {
            parts.push_back(path.substr(start, slash - start));
        }
        start = slash + 1;
    }
    return parts;
}

class Extractor {
public:
    Extractor(const fs::path& destination, const ExtractRules& rules) : rules_(rules) {
        std::error_code error;
        fs::create_directories(destination, error);
        root_ = fs::weakly_canonical(destination, error);
    }

    [[nodiscard]] std::string failure() const { return failure_; }

    bool directory(std::string_view entry) {
        const auto target = place(entry);
        if (!target.has_value() || !target->has_value()) {
            return target.has_value();
        }
        if (!guard(**target)) {
            return false;
        }
        std::error_code error;
        fs::create_directories(**target, error);
        return true;
    }

    template <typename Writer>
    bool file(std::string_view entry, std::uint32_t mode, Writer&& write) {
        const auto target = place(entry);
        if (!target.has_value()) {
            return false;
        }
        if (!target->has_value()) {
            return write(nullptr);
        }
        const fs::path& path = **target;
        if (!guard(path.parent_path()) || ++entries_ > kMaxEntries) {
            return fail("too many entries or an entry outside the destination: " + std::string{entry});
        }
        std::error_code error;
        fs::create_directories(path.parent_path(), error);
        fs::remove(path, error);
        std::ofstream out{path, std::ios::binary};
        if (!out) {
            return fail("cannot write " + path.string());
        }
        if (!write(&out)) {
            return false;
        }
        out.close();
        applyMode(path, mode);
        return true;
    }

    bool link(std::string_view entry, const std::string& target, bool hard) {
        const auto place_ = place(entry);
        if (!place_.has_value()) {
            return false;
        }
        if (!place_->has_value()) {
            return true;
        }
        const fs::path& path = **place_;
        if (!guard(path.parent_path())) {
            return false;
        }
        std::error_code error;
        fs::create_directories(path.parent_path(), error);
        fs::remove(path, error);
        if (hard) {
            const auto source = place(target);
            if (!source.has_value() || !source->has_value() || !guard((*source)->parent_path())) {
                return fail("hard link outside the destination: " + std::string{entry});
            }
            fs::copy_file(**source, path, error);
            return !error || fail("cannot copy a hard link target for " + std::string{entry});
        }
        const fs::path value{target};
        if (value.is_absolute() || value.has_root_name() ||
            !isWithin(root_, fs::weakly_canonical(path.parent_path() / value, error))) {
            return fail("symlink " + std::string{entry} + " points outside the destination");
        }
#if defined(_WIN32)
        return true;
#else
        fs::create_symlink(value, path, error);
        return !error || fail("cannot create the symlink " + std::string{entry});
#endif
    }

    std::uint64_t extracted = 0;

private:
    bool fail(std::string why) {
        failure_ = std::move(why);
        return false;
    }

    std::expected<std::optional<fs::path>, std::string> place(std::string_view entry) {
        auto result = entryDestination(entry, rules_);
        if (!result.has_value()) {
            failure_ = result.error();
            return std::unexpected(result.error());
        }
        if (!result->has_value()) {
            return std::optional<fs::path>{};
        }
        return std::optional<fs::path>{root_ / **result};
    }

    bool guard(const fs::path& path) {
        std::error_code error;
        const fs::path resolved = fs::weakly_canonical(path, error);
        if (error || (resolved != root_ && !isWithin(root_, resolved))) {
            return fail("an archive entry would land outside the destination: " + path.string());
        }
        return true;
    }

    static void applyMode(const fs::path& path, std::uint32_t mode) {
#if !defined(_WIN32)
        std::error_code error;
        fs::permissions(path, static_cast<fs::perms>((mode & 0777) | 0600), fs::perm_options::replace, error);
#else
        (void)path;
        (void)mode;
#endif
    }

    ExtractRules rules_;
    fs::path root_;
    std::string failure_;
    std::size_t entries_ = 0;
};

std::uint64_t octal(const std::uint8_t* field, std::size_t size) {
    if ((field[0] & 0x80) != 0) {
        std::uint64_t value = field[0] & 0x7F;
        for (std::size_t i = 1; i < size; ++i) {
            value = (value << 8) | field[i];
        }
        return value;
    }
    std::uint64_t value = 0;
    for (std::size_t i = 0; i < size && field[i] >= '0' && field[i] <= '7'; ++i) {
        value = value * 8 + (field[i] - '0');
    }
    return value;
}

std::string text(const std::uint8_t* field, std::size_t size) {
    const char* start = reinterpret_cast<const char*>(field);
    return std::string{start, strnlen(start, size)};
}

bool readExactly(ByteSource& source, std::uint8_t* out, std::size_t size) {
    std::size_t done = 0;
    while (done < size) {
        const std::size_t count = source.read({out + done, size - done});
        if (count == 0) {
            return false;
        }
        done += count;
    }
    return true;
}

bool skipBytes(ByteSource& source, std::uint64_t size) {
    std::array<std::uint8_t, 8192> sink{};
    while (size > 0) {
        const std::size_t count =
            source.read({sink.data(), static_cast<std::size_t>(std::min<std::uint64_t>(size, sink.size()))});
        if (count == 0) {
            return false;
        }
        size -= count;
    }
    return true;
}

bool checksumHolds(const std::array<std::uint8_t, kBlock>& header) {
    std::uint64_t sum = 0;
    for (std::size_t i = 0; i < kBlock; ++i) {
        sum += (i >= 148 && i < 156) ? ' ' : header[i];
    }
    return sum == octal(header.data() + 148, 8);
}

void applyPax(const std::string& records, std::string& path, std::string& linkPath) {
    std::size_t at = 0;
    while (at < records.size()) {
        const std::size_t space = records.find(' ', at);
        std::size_t length = 0;
        if (space == std::string::npos ||
            std::from_chars(records.data() + at, records.data() + space, length).ec != std::errc{} || length == 0 ||
            at + length > records.size()) {
            return;
        }
        const std::string record = records.substr(space + 1, at + length - space - 2);
        if (record.starts_with("path=")) {
            path = record.substr(5);
        } else if (record.starts_with("linkpath=")) {
            linkPath = record.substr(9);
        }
        at += length;
    }
}

std::expected<void, std::string> extractTar(ByteSource& source, Extractor& extractor) {
    std::string longName;
    std::string longLink;
    std::string paxPath;
    std::string paxLink;
    for (;;) {
        std::array<std::uint8_t, kBlock> header{};
        if (!readExactly(source, header.data(), kBlock)) {
            if (!source.error.empty()) {
                return std::unexpected(source.error);
            }
            return {};
        }
        if (std::ranges::all_of(header, [](std::uint8_t b) { return b == 0; })) {
            return {};
        }
        if (!checksumHolds(header)) {
            return std::unexpected(source.error.empty() ? "the tar header checksum does not match" : source.error);
        }
        const char type = static_cast<char>(header[156]);
        std::string name = text(header.data(), 100);
        if (std::memcmp(header.data() + 257, "ustar", 5) == 0) {
            if (const std::string prefix = text(header.data() + 345, 155); !prefix.empty()) {
                name = prefix + "/" + name;
            }
        }
        std::string link = text(header.data() + 157, 100);
        const std::uint64_t size = octal(header.data() + 124, 12);
        const auto mode = static_cast<std::uint32_t>(octal(header.data() + 100, 8));
        const std::uint64_t padded = (size + kBlock - 1) / kBlock * kBlock;
        extractor.extracted += size;
        if (extractor.extracted > kMaxExtractedBytes) {
            return std::unexpected("the archive expands to more than 8 GiB");
        }
        if (type == 'x' || type == 'L' || type == 'K') {
            if (size > kMaxMetadataBytes) {
                return std::unexpected("oversized tar metadata entry");
            }
            std::string data(static_cast<std::size_t>(padded), '\0');
            if (!readExactly(source, reinterpret_cast<std::uint8_t*>(data.data()), data.size())) {
                return std::unexpected("truncated tar metadata entry");
            }
            data.resize(static_cast<std::size_t>(size));
            if (type == 'x') {
                applyPax(data, paxPath, paxLink);
            } else if (type == 'L') {
                longName = text(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());
            } else {
                longLink = text(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());
            }
            continue;
        }
        if (!paxPath.empty()) {
            name = paxPath;
        } else if (!longName.empty()) {
            name = longName;
        }
        if (!paxLink.empty()) {
            link = paxLink;
        } else if (!longLink.empty()) {
            link = longLink;
        }
        longName.clear();
        longLink.clear();
        paxPath.clear();
        paxLink.clear();
        bool ok = true;
        if (type == '5') {
            ok = extractor.directory(name) && skipBytes(source, padded);
        } else if (type == '0' || type == '\0' || type == '7') {
            std::uint64_t left = size;
            ok = extractor.file(name, mode, [&](std::ofstream* out) {
                std::array<std::uint8_t, 65536> chunk{};
                while (left > 0) {
                    const std::size_t want = static_cast<std::size_t>(std::min<std::uint64_t>(left, chunk.size()));
                    if (!readExactly(source, chunk.data(), want)) {
                        return false;
                    }
                    if (out != nullptr) {
                        out->write(reinterpret_cast<const char*>(chunk.data()), static_cast<std::streamsize>(want));
                    }
                    left -= want;
                }
                return out == nullptr || static_cast<bool>(*out);
            });
            ok = ok && skipBytes(source, padded - size);
        } else if (type == '2' || type == '1') {
            ok = extractor.link(name, link, type == '1') && skipBytes(source, padded);
        } else {
            ok = skipBytes(source, padded);
        }
        if (!ok) {
            const std::string why = !extractor.failure().empty() ? extractor.failure() : source.error;
            return std::unexpected(why.empty() ? "truncated or corrupt archive at " + name : why);
        }
    }
}

std::expected<void, std::string> extractZip(const fs::path& archive, Extractor& extractor) {
    auto bytes = wire::io::fileIO::readBytes(archive.string());
    if (!bytes.has_value()) {
        return std::unexpected("cannot read " + archive.string());
    }
    auto zip = wire::io::ZipReader::open(std::move(*bytes));
    if (!zip.has_value()) {
        return std::unexpected("not a readable zip archive: " + archive.string());
    }
    for (const std::string& name : zip->entryNames()) {
        if (name.find('\\') != std::string::npos) {
            return std::unexpected("zip entry with a backslash: " + name);
        }
        if (name.ends_with('/')) {
            if (!extractor.directory(name)) {
                return std::unexpected(extractor.failure());
            }
            continue;
        }
        const auto data = zip->read(name);
        if (!data.has_value()) {
            return std::unexpected("cannot decompress " + name);
        }
        extractor.extracted += data->size();
        if (extractor.extracted > kMaxExtractedBytes) {
            return std::unexpected("the archive expands to more than 8 GiB");
        }
        const bool ok = extractor.file(name, 0755, [&](std::ofstream* out) {
            if (out != nullptr) {
                out->write(reinterpret_cast<const char*>(data->data()), static_cast<std::streamsize>(data->size()));
            }
            return out == nullptr || static_cast<bool>(*out);
        });
        if (!ok) {
            return std::unexpected(extractor.failure().empty() ? "cannot write " + name : extractor.failure());
        }
    }
    return {};
}

} // namespace

std::unique_ptr<ByteSource> openFileSource(const fs::path& path) { return std::make_unique<FileSource>(path); }

std::unique_ptr<ByteSource> openGzipSource(const fs::path& path) { return std::make_unique<GzipSource>(path); }

std::expected<std::optional<fs::path>, std::string> entryDestination(std::string_view entry,
                                                                     const ExtractRules& rules) {
    if (entry.find('\\') != std::string_view::npos || entry.find('\0') != std::string_view::npos ||
        entry.starts_with("/")) {
        return std::unexpected("unsafe archive path: " + std::string{entry});
    }
    std::vector<std::string_view> parts = components(entry);
    for (const std::string_view part : parts) {
        if (part == ".." || (part.size() == 2 && part[1] == ':')) {
            return std::unexpected("unsafe archive path: " + std::string{entry});
        }
    }
    std::erase(parts, std::string_view{"."});
    if (parts.size() <= rules.skip) {
        return std::optional<fs::path>{};
    }
    parts.erase(parts.begin(), parts.begin() + static_cast<std::ptrdiff_t>(rules.skip));
    if (!rules.only.empty()) {
        const std::vector<std::string_view> prefix = components(rules.only);
        if (parts.size() <= prefix.size() || !std::equal(prefix.begin(), prefix.end(), parts.begin())) {
            return std::optional<fs::path>{};
        }
        parts.erase(parts.begin(), parts.begin() + static_cast<std::ptrdiff_t>(prefix.size()));
    }
    fs::path result;
    for (const std::string_view part : parts) {
        result /= std::string{part};
    }
    return std::optional<fs::path>{result};
}

std::expected<void, std::string> extractArchive(const fs::path& archive, ArchiveFormat format,
                                                const fs::path& destination, const ExtractRules& rules) {
    Extractor extractor{destination, rules};
    if (format == ArchiveFormat::Zip) {
        return extractZip(archive, extractor);
    }
    const std::unique_ptr<ByteSource> source =
        format == ArchiveFormat::TarGz ? openGzipSource(archive) : openXzSource(archive);
    if (!source->error.empty()) {
        return std::unexpected(source->error);
    }
    return extractTar(*source, extractor);
}

} // namespace basalt::kiln
