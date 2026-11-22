#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <memory>
#include <span>
#include <string>

namespace basalt::kiln {

class Sha256 {
public:
    Sha256();
    Sha256(const Sha256&) = delete;
    Sha256& operator=(const Sha256&) = delete;
    ~Sha256();

    void update(std::span<const std::uint8_t> bytes);

    /// Lower case hex; the object cannot be reused afterwards.
    [[nodiscard]] std::string finishHex();

private:
    struct State;

    std::unique_ptr<State> state_;
};

[[nodiscard]] std::expected<std::string, std::string> sha256OfFile(const std::filesystem::path& path);

} // namespace basalt::kiln
