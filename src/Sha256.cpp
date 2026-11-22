#include "Sha256.hpp"

#include <array>
#include <fstream>
#include <mbedtls/sha256.h>

namespace basalt::kiln {

struct Sha256::State {
    mbedtls_sha256_context context;
};

Sha256::Sha256() : state_(std::make_unique<State>()) {
    mbedtls_sha256_init(&state_->context);
    mbedtls_sha256_starts(&state_->context, 0);
}

Sha256::~Sha256() { mbedtls_sha256_free(&state_->context); }

void Sha256::update(std::span<const std::uint8_t> bytes) {
    mbedtls_sha256_update(&state_->context, bytes.data(), bytes.size());
}

std::string Sha256::finishHex() {
    std::array<std::uint8_t, 32> digest{};
    mbedtls_sha256_finish(&state_->context, digest.data());
    constexpr std::string_view kDigits = "0123456789abcdef";
    std::string text;
    for (const std::uint8_t byte : digest) {
        text += kDigits[byte >> 4];
        text += kDigits[byte & 0x0F];
    }
    return text;
}

std::expected<std::string, std::string> sha256OfFile(const std::filesystem::path& path) {
    std::ifstream in{path, std::ios::binary};
    if (!in) {
        return std::unexpected("cannot read " + path.string());
    }
    Sha256 hash;
    std::array<char, 65536> buffer{};
    while (in.read(buffer.data(), buffer.size()) || in.gcount() > 0) {
        hash.update({reinterpret_cast<const std::uint8_t*>(buffer.data()), static_cast<std::size_t>(in.gcount())});
    }
    return hash.finishHex();
}

} // namespace basalt::kiln
