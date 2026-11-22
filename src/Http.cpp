#include "Http.hpp"

#include <basalt_mbedtls_threads.h>

extern "C" {
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#include <mbedtls/x509_crt.h>
}

#if defined(_WIN32)
#include <wincrypt.h>
#include <windows.h>
#endif

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <cstdlib>
#include <fstream>
#include <map>
#include <optional>
#include <thread>

namespace fs = std::filesystem;

namespace basalt::kiln {
namespace {

constexpr int kMaxRedirects = 5;
constexpr int kMaxAttempts = 4;
constexpr std::size_t kMaxHeaderBytes = 64UL * 1024UL;
constexpr std::uint32_t kReadTimeoutMs = 60000;

constexpr std::array<const char*, 6> kTrustStores{
    "/etc/ssl/cert.pem",
    "/etc/ssl/certs/ca-certificates.crt",
    "/etc/pki/tls/certs/ca-bundle.crt",
    "/etc/ssl/ca-bundle.pem",
    "/etc/pki/tls/cacert.pem",
    "/usr/local/share/certs/ca-root-nss.crt",
};

std::string describe(int code) {
    std::array<char, 128> text{};
    mbedtls_strerror(code, text.data(), text.size());
    return std::string{text.data()};
}

struct Url {
    std::string host;
    std::string port = "443";
    std::string path = "/";
};

std::optional<Url> parseUrl(std::string_view url) {
    constexpr std::string_view kScheme = "https://";
    if (!url.starts_with(kScheme) || std::ranges::any_of(url, [](unsigned char c) { return c <= 32 || c == 127; })) {
        return std::nullopt;
    }
    url.remove_prefix(kScheme.size());
    url = url.substr(0, url.find('#'));
    const std::size_t boundary = url.find_first_of("/?");
    std::string_view authority = url.substr(0, boundary);
    if (authority.empty() || authority.find('@') != std::string_view::npos || authority.front() == '[') {
        return std::nullopt;
    }
    Url parsed;
    if (boundary != std::string_view::npos) {
        parsed.path = std::string{url.substr(boundary)};
        if (parsed.path.front() == '?') {
            parsed.path.insert(parsed.path.begin(), '/');
        }
    }
    if (const std::size_t colon = authority.find(':'); colon != std::string_view::npos) {
        parsed.port = std::string{authority.substr(colon + 1)};
        authority = authority.substr(0, colon);
    }
    parsed.host = std::string{authority};
    return parsed.host.empty() || parsed.port.empty() ? std::nullopt : std::optional<Url>{parsed};
}

class Trust {
public:
    Trust() {
        basalt_mbedtls_threading_init();
        mbedtls_x509_crt_init(&roots_);
    }

    Trust(const Trust&) = delete;
    Trust& operator=(const Trust&) = delete;

    ~Trust() { mbedtls_x509_crt_free(&roots_); }

    [[nodiscard]] bool load(std::string& error) {
        if (const char* chosen = std::getenv("SSL_CERT_FILE"); chosen != nullptr && *chosen != '\0') {
            if (mbedtls_x509_crt_parse_file(&roots_, chosen) >= 0) {
                return true;
            }
        }
#if defined(_WIN32)
        if (HCERTSTORE store = CertOpenSystemStoreA(0, "ROOT"); store != nullptr) {
            for (PCCERT_CONTEXT certificate = CertEnumCertificatesInStore(store, nullptr); certificate != nullptr;
                 certificate = CertEnumCertificatesInStore(store, certificate)) {
                mbedtls_x509_crt_parse_der(&roots_, certificate->pbCertEncoded, certificate->cbCertEncoded);
            }
            CertCloseStore(store, 0);
            return true;
        }
#endif
        for (const char* path : kTrustStores) {
            if (fs::exists(path) && mbedtls_x509_crt_parse_file(&roots_, path) >= 0) {
                return true;
            }
        }
        error = "no CA certificate bundle found; set SSL_CERT_FILE to one";
        return false;
    }

    [[nodiscard]] mbedtls_x509_crt* roots() { return &roots_; }

private:
    mbedtls_x509_crt roots_{};
};

class Connection {
public:
    explicit Connection(Trust& trust) : trust_(trust) {
        mbedtls_net_init(&net_);
        mbedtls_ssl_init(&ssl_);
        mbedtls_ssl_config_init(&config_);
        mbedtls_ctr_drbg_init(&random_);
        mbedtls_entropy_init(&entropy_);
    }

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    ~Connection() {
        mbedtls_ssl_close_notify(&ssl_);
        mbedtls_net_free(&net_);
        mbedtls_ssl_free(&ssl_);
        mbedtls_ssl_config_free(&config_);
        mbedtls_ctr_drbg_free(&random_);
        mbedtls_entropy_free(&entropy_);
    }

    [[nodiscard]] bool open(const Url& url, std::string& error) {
        int code = mbedtls_ctr_drbg_seed(&random_, mbedtls_entropy_func, &entropy_, nullptr, 0);
        if (code == 0) {
            code = mbedtls_ssl_config_defaults(&config_, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM,
                                               MBEDTLS_SSL_PRESET_DEFAULT);
        }
        if (code != 0) {
            error = "tls setup failed: " + describe(code);
            return false;
        }
        mbedtls_ssl_conf_authmode(&config_, MBEDTLS_SSL_VERIFY_REQUIRED);
        mbedtls_ssl_conf_ca_chain(&config_, trust_.roots(), nullptr);
        mbedtls_ssl_conf_rng(&config_, mbedtls_ctr_drbg_random, &random_);
        mbedtls_ssl_conf_read_timeout(&config_, kReadTimeoutMs);
        mbedtls_ssl_conf_min_tls_version(&config_, MBEDTLS_SSL_VERSION_TLS1_2);
        if ((code = mbedtls_ssl_setup(&ssl_, &config_)) != 0 ||
            (code = mbedtls_ssl_set_hostname(&ssl_, url.host.c_str())) != 0) {
            error = "tls setup failed: " + describe(code);
            return false;
        }
        if ((code = mbedtls_net_connect(&net_, url.host.c_str(), url.port.c_str(), MBEDTLS_NET_PROTO_TCP)) != 0) {
            error = "cannot connect to " + url.host + ": " + describe(code);
            return false;
        }
        mbedtls_ssl_set_bio(&ssl_, &net_, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);
        while ((code = mbedtls_ssl_handshake(&ssl_)) != 0) {
            if (code != MBEDTLS_ERR_SSL_WANT_READ && code != MBEDTLS_ERR_SSL_WANT_WRITE) {
                error = "tls handshake with " + url.host + " failed: " + describe(code);
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] bool writeAll(std::string_view text) {
        while (!text.empty()) {
            const int written =
                mbedtls_ssl_write(&ssl_, reinterpret_cast<const unsigned char*>(text.data()), text.size());
            if (written == MBEDTLS_ERR_SSL_WANT_READ || written == MBEDTLS_ERR_SSL_WANT_WRITE) {
                continue;
            }
            if (written <= 0) {
                return false;
            }
            text.remove_prefix(static_cast<std::size_t>(written));
        }
        return true;
    }

    /// Bytes read, 0 at the end of the stream, a negative value on failure.
    [[nodiscard]] int read(std::uint8_t* out, std::size_t size) {
        for (;;) {
            const int count = mbedtls_ssl_read(&ssl_, out, size);
            if (count == MBEDTLS_ERR_SSL_WANT_READ || count == MBEDTLS_ERR_SSL_WANT_WRITE) {
                continue;
            }
            if (count == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY || count == MBEDTLS_ERR_NET_CONN_RESET) {
                return 0;
            }
            return count;
        }
    }

private:
    Trust& trust_;
    mbedtls_net_context net_{};
    mbedtls_ssl_context ssl_{};
    mbedtls_ssl_config config_{};
    mbedtls_ctr_drbg_context random_{};
    mbedtls_entropy_context entropy_{};
};

struct Response {
    int status = 0;
    std::map<std::string, std::string> headers;
    std::string leftover;
};

std::string lowered(std::string text) {
    std::ranges::transform(text, text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

std::string trimmed(std::string_view text) {
    while (!text.empty() && (text.front() == ' ' || text.front() == '\t')) {
        text.remove_prefix(1);
    }
    while (!text.empty() && (text.back() == ' ' || text.back() == '\t' || text.back() == '\r')) {
        text.remove_suffix(1);
    }
    return std::string{text};
}

std::optional<Response> readHead(Connection& connection, std::string& error) {
    std::string head;
    std::array<std::uint8_t, 4096> buffer{};
    std::size_t end = std::string::npos;
    while (end == std::string::npos) {
        const int count = connection.read(buffer.data(), buffer.size());
        if (count <= 0 || head.size() > kMaxHeaderBytes) {
            error = "the server closed the connection before answering";
            return std::nullopt;
        }
        head.append(reinterpret_cast<const char*>(buffer.data()), static_cast<std::size_t>(count));
        end = head.find("\r\n\r\n");
    }
    Response response;
    response.leftover = head.substr(end + 4);
    head.resize(end);
    std::size_t lineEnd = head.find("\r\n");
    const std::string statusLine = head.substr(0, lineEnd);
    const std::size_t space = statusLine.find(' ');
    if (!statusLine.starts_with("HTTP/1.") || space == std::string::npos ||
        std::from_chars(statusLine.data() + space + 1, statusLine.data() + statusLine.size(), response.status).ec !=
            std::errc{}) {
        error = "malformed HTTP status line";
        return std::nullopt;
    }
    while (lineEnd != std::string::npos) {
        const std::size_t start = lineEnd + 2;
        lineEnd = head.find("\r\n", start);
        const std::string line = head.substr(start, lineEnd == std::string::npos ? std::string::npos : lineEnd - start);
        if (const std::size_t colon = line.find(':'); colon != std::string::npos) {
            response.headers[lowered(line.substr(0, colon))] = trimmed(line.substr(colon + 1));
        }
    }
    return response;
}

std::optional<std::uint64_t> parseNumber(const std::string& text, int base = 10) {
    std::uint64_t value = 0;
    const auto [end, code] = std::from_chars(text.data(), text.data() + text.size(), value, base);
    return code == std::errc{} && end != text.data() ? std::optional<std::uint64_t>{value} : std::nullopt;
}

enum class Outcome { Done, Redirect, Retry, Fail };

struct Attempt {
    Outcome outcome = Outcome::Fail;
    std::string message;
    std::string location;
};

/// Reads the response body and writes it to `out`; `stored` counts what the file holds when the call returns.
class BodyReader {
public:
    BodyReader(Connection& connection, std::string leftover) : connection_(connection), pending_(std::move(leftover)) {}

    /// Next piece of the stream into `out`; false at the end or on failure (`failed` tells which).
    bool next(std::string& out, std::size_t limit) {
        out.clear();
        if (!pending_.empty()) {
            out = pending_.substr(0, limit);
            pending_.erase(0, out.size());
            return true;
        }
        std::array<std::uint8_t, 65536> buffer{};
        const int count = connection_.read(buffer.data(), std::min(limit, buffer.size()));
        if (count < 0) {
            failed = true;
            return false;
        }
        out.assign(reinterpret_cast<const char*>(buffer.data()), static_cast<std::size_t>(count));
        return count > 0;
    }

    /// One line without its terminator, for chunk headers.
    std::optional<std::string> line() {
        std::string collected;
        std::string piece;
        for (;;) {
            const std::size_t newline = pending_.find('\n');
            if (newline != std::string::npos) {
                collected += pending_.substr(0, newline);
                pending_.erase(0, newline + 1);
                return trimmed(collected);
            }
            collected += pending_;
            pending_.clear();
            std::array<std::uint8_t, 1024> buffer{};
            const int count = connection_.read(buffer.data(), buffer.size());
            if (count <= 0 || collected.size() > 4096) {
                failed = true;
                return std::nullopt;
            }
            pending_.assign(reinterpret_cast<const char*>(buffer.data()), static_cast<std::size_t>(count));
        }
    }

    bool failed = false;

private:
    Connection& connection_;
    std::string pending_;
};

Attempt fetchOnce(Trust& trust, const Url& url, const fs::path& part, const ProgressFn& progress) {
    Attempt attempt;
    std::uint64_t existing = 0;
    std::error_code ignored;
    if (fs::exists(part, ignored)) {
        existing = fs::file_size(part, ignored);
    }
    Connection connection{trust};
    std::string error;
    if (!connection.open(url, error)) {
        attempt = {Outcome::Retry, error, {}};
        return attempt;
    }
    std::string request = "GET " + url.path + " HTTP/1.1\r\nHost: " + url.host +
                          "\r\nUser-Agent: kiln\r\nAccept: */*\r\nConnection: close\r\n";
    if (existing > 0) {
        request += "Range: bytes=" + std::to_string(existing) + "-\r\n";
    }
    request += "\r\n";
    if (!connection.writeAll(request)) {
        return {Outcome::Retry, "could not send the request to " + url.host, {}};
    }
    const std::optional<Response> head = readHead(connection, error);
    if (!head.has_value()) {
        return {Outcome::Retry, error, {}};
    }
    const int status = head->status;
    if (status == 301 || status == 302 || status == 303 || status == 307 || status == 308) {
        const auto location = head->headers.find("location");
        if (location == head->headers.end()) {
            return {Outcome::Fail, "redirect without a Location header", {}};
        }
        return {Outcome::Redirect, {}, location->second};
    }
    if (status == 416) {
        fs::remove(part, ignored);
        return {Outcome::Retry, "the partial download no longer matches the server; starting over", {}};
    }
    if (status != 200 && status != 206) {
        const bool transient = status >= 500 || status == 429;
        return {transient ? Outcome::Retry : Outcome::Fail,
                "HTTP " + std::to_string(status) + " from " + url.host + url.path,
                {}};
    }
    std::uint64_t offset = 0;
    if (status == 206) {
        const auto range = head->headers.find("content-range");
        const std::string_view prefix = "bytes ";
        if (range == head->headers.end() || !range->second.starts_with(prefix) ||
            parseNumber(range->second.substr(prefix.size(), range->second.find('-') - prefix.size())) != existing) {
            fs::remove(part, ignored);
            return {Outcome::Retry, "the server answered a resume with the wrong range", {}};
        }
        offset = existing;
    }
    std::optional<std::uint64_t> length;
    if (const auto found = head->headers.find("content-length"); found != head->headers.end()) {
        length = parseNumber(found->second);
    }
    const auto encoding = head->headers.find("transfer-encoding");
    const bool chunked =
        encoding != head->headers.end() && lowered(encoding->second).find("chunked") != std::string::npos;
    std::ofstream out{part, std::ios::binary | (offset > 0 ? std::ios::app : std::ios::trunc)};
    if (!out) {
        return {Outcome::Fail, "cannot write " + part.string(), {}};
    }
    const std::uint64_t total = length.has_value() ? offset + *length : 0;
    std::uint64_t stored = offset;
    BodyReader body{connection, head->leftover};
    std::string piece;
    const auto store = [&](const std::string& bytes) {
        out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
        stored += bytes.size();
        if (progress) {
            progress(stored, chunked ? 0 : total);
        }
        return static_cast<bool>(out);
    };
    if (chunked) {
        for (;;) {
            const std::optional<std::string> sizeLine = body.line();
            const std::optional<std::uint64_t> size =
                sizeLine.has_value() ? parseNumber(sizeLine->substr(0, sizeLine->find(';')), 16) : std::nullopt;
            if (!size.has_value()) {
                return {Outcome::Retry, "malformed chunked response", {}};
            }
            if (*size == 0) {
                break;
            }
            std::uint64_t left = *size;
            while (left > 0) {
                if (!body.next(piece, static_cast<std::size_t>(std::min<std::uint64_t>(left, 65536))) ||
                    !store(piece)) {
                    return {Outcome::Retry, "the connection dropped during the download", {}};
                }
                left -= piece.size();
            }
            body.line();
        }
    } else {
        std::uint64_t left = length.value_or(UINT64_MAX);
        while (left > 0 && body.next(piece, static_cast<std::size_t>(std::min<std::uint64_t>(left, 65536)))) {
            if (!store(piece)) {
                return {Outcome::Fail, "cannot write " + part.string(), {}};
            }
            left -= piece.size();
        }
        if (body.failed || (length.has_value() && left > 0)) {
            return {Outcome::Retry, "the connection dropped during the download", {}};
        }
    }
    out.close();
    return {Outcome::Done, {}, {}};
}

std::optional<Url> resolveLocation(const Url& base, const std::string& location) {
    if (location.starts_with("/")) {
        Url next = base;
        next.path = location;
        return next;
    }
    return parseUrl(location);
}

} // namespace

std::expected<void, std::string> httpsDownload(std::string_view url, const fs::path& part, const ProgressFn& progress) {
    std::optional<Url> current = parseUrl(url);
    if (!current.has_value()) {
        return std::unexpected("only https URLs can be downloaded: " + std::string{url});
    }
    Trust trust;
    std::string error;
    if (!trust.load(error)) {
        return std::unexpected(error);
    }
    fs::create_directories(part.parent_path());
    int redirects = 0;
    int failures = 0;
    for (;;) {
        const Attempt attempt = fetchOnce(trust, *current, part, progress);
        switch (attempt.outcome) {
        case Outcome::Done:
            return {};
        case Outcome::Redirect: {
            current = resolveLocation(*current, attempt.location);
            if (!current.has_value() || ++redirects > kMaxRedirects) {
                return std::unexpected("too many redirects or a redirect to a non-https address");
            }
            std::error_code ignored;
            fs::remove(part, ignored);
            break;
        }
        case Outcome::Retry:
            if (++failures >= kMaxAttempts) {
                return std::unexpected(attempt.message);
            }
            std::this_thread::sleep_for(std::chrono::seconds{failures});
            break;
        case Outcome::Fail:
            return std::unexpected(attempt.message);
        }
    }
}

} // namespace basalt::kiln
