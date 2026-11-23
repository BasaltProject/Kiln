#include "ByteSource.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <optional>
#include <vector>

namespace basalt::kiln {
namespace {

constexpr std::size_t kMaxDictionary = 1UL << 30;
constexpr int kStates = 12;
constexpr int kPosStates = 16;
constexpr std::uint16_t kInitialProbability = 1024;

struct LengthCoder {
    std::uint16_t choice = kInitialProbability;
    std::uint16_t choice2 = kInitialProbability;
    std::array<std::array<std::uint16_t, 8>, kPosStates> low;
    std::array<std::array<std::uint16_t, 8>, kPosStates> mid;
    std::array<std::uint16_t, 256> high;

    void reset() {
        choice = choice2 = kInitialProbability;
        for (auto& row : low) {
            row.fill(kInitialProbability);
        }
        for (auto& row : mid) {
            row.fill(kInitialProbability);
        }
        high.fill(kInitialProbability);
    }
};

class RangeDecoder {
public:
    bool start(const std::uint8_t* data, std::size_t size) {
        data_ = data;
        end_ = data + size;
        if (size < 5 || data[0] != 0) {
            return false;
        }
        range_ = 0xFFFFFFFFU;
        code_ = (static_cast<std::uint32_t>(data[1]) << 24) | (static_cast<std::uint32_t>(data[2]) << 16) |
                (static_cast<std::uint32_t>(data[3]) << 8) | data[4];
        data_ += 5;
        overrun_ = false;
        return true;
    }

    [[nodiscard]] bool overrun() const { return overrun_; }

    int bit(std::uint16_t& probability) {
        normalize();
        const std::uint32_t bound = (range_ >> 11) * probability;
        if (code_ < bound) {
            range_ = bound;
            probability = static_cast<std::uint16_t>(probability + ((2048 - probability) >> 5));
            return 0;
        }
        range_ -= bound;
        code_ -= bound;
        probability = static_cast<std::uint16_t>(probability - (probability >> 5));
        return 1;
    }

    std::uint32_t direct(int count) {
        std::uint32_t result = 0;
        while (count-- > 0) {
            normalize();
            range_ >>= 1;
            std::uint32_t taken = 0;
            if (code_ >= range_) {
                code_ -= range_;
                taken = 1;
            }
            result = (result << 1) | taken;
        }
        return result;
    }

    std::uint32_t tree(std::uint16_t* probabilities, int bits) {
        std::uint32_t m = 1;
        for (int i = 0; i < bits; ++i) {
            m = (m << 1) | static_cast<std::uint32_t>(bit(probabilities[m]));
        }
        return m - (1U << bits);
    }

    std::uint32_t reverseTree(std::uint16_t* probabilities, int bits) {
        std::uint32_t m = 1;
        std::uint32_t result = 0;
        for (int i = 0; i < bits; ++i) {
            const auto taken = static_cast<std::uint32_t>(bit(probabilities[m]));
            m = (m << 1) | taken;
            result |= taken << i;
        }
        return result;
    }

private:
    void normalize() {
        if (range_ < (1U << 24)) {
            range_ <<= 8;
            if (data_ >= end_) {
                overrun_ = true;
                code_ <<= 8;
            } else {
                code_ = (code_ << 8) | *data_++;
            }
        }
    }

    const std::uint8_t* data_ = nullptr;
    const std::uint8_t* end_ = nullptr;
    std::uint32_t range_ = 0;
    std::uint32_t code_ = 0;
    bool overrun_ = false;
};

class Lzma2Decoder {
public:
    void setDictionary(std::size_t size) {
        window_.assign(std::max<std::size_t>(size, 4096), 0);
        position_ = 0;
        filled_ = 0;
    }

    void resetDictionary() {
        position_ = 0;
        filled_ = 0;
    }

    void appendRaw(const std::uint8_t* data, std::size_t size, std::vector<std::uint8_t>& out) {
        for (std::size_t i = 0; i < size; ++i) {
            put(data[i], out);
        }
    }

    bool setProperties(std::uint8_t properties) {
        if (properties >= 9 * 5 * 5) {
            return false;
        }
        lc_ = properties % 9;
        properties /= 9;
        lp_ = properties % 5;
        pb_ = properties / 5;
        if (lc_ + lp_ > 4) {
            return false;
        }
        literal_.assign(static_cast<std::size_t>(0x300) << (lc_ + lp_), kInitialProbability);
        return true;
    }

    void resetState() {
        state_ = 0;
        rep0_ = rep1_ = rep2_ = rep3_ = 0;
        std::ranges::fill(literal_, kInitialProbability);
        for (auto& row : isMatch_) {
            row.fill(kInitialProbability);
        }
        for (auto& row : isRep0Long_) {
            row.fill(kInitialProbability);
        }
        isRep_.fill(kInitialProbability);
        isRepG0_.fill(kInitialProbability);
        isRepG1_.fill(kInitialProbability);
        isRepG2_.fill(kInitialProbability);
        for (auto& row : posSlot_) {
            row.fill(kInitialProbability);
        }
        posSpecial_.fill(kInitialProbability);
        align_.fill(kInitialProbability);
        matchLength_.reset();
        repLength_.reset();
    }

    /// Decodes one LZMA chunk that expands to `unpacked` bytes.
    bool decode(const std::uint8_t* data, std::size_t size, std::size_t unpacked, std::vector<std::uint8_t>& out) {
        if (!range_.start(data, size) || literal_.empty()) {
            return false;
        }
        const std::uint32_t posMask = (1U << pb_) - 1;
        const std::uint32_t literalMask = (1U << lp_) - 1;
        std::size_t produced = 0;
        while (produced < unpacked) {
            const auto total = static_cast<std::uint32_t>(totalPosition_);
            const std::uint32_t posState = total & posMask;
            if (range_.bit(isMatch_[static_cast<std::size_t>(state_)][posState]) == 0) {
                const std::uint8_t previous =
                    filled_ > 0 ? window_[(position_ + window_.size() - 1) % window_.size()] : 0;
                std::uint16_t* probabilities =
                    literal_.data() +
                    0x300 * (((total & literalMask) << lc_) + (static_cast<std::uint32_t>(previous) >> (8 - lc_)));
                std::uint32_t symbol = 1;
                if (state_ >= 7) {
                    std::uint32_t matchByte = byteAt(rep0_ + 1);
                    std::uint32_t offset = 0x100;
                    while (symbol < 0x100) {
                        matchByte <<= 1;
                        const std::uint32_t matchBit = matchByte & offset;
                        const int bit = range_.bit(probabilities[offset + matchBit + symbol]);
                        symbol = (symbol << 1) | static_cast<std::uint32_t>(bit);
                        offset &= bit != 0 ? matchBit : ~matchBit;
                    }
                } else {
                    while (symbol < 0x100) {
                        symbol = (symbol << 1) | static_cast<std::uint32_t>(range_.bit(probabilities[symbol]));
                    }
                }
                put(static_cast<std::uint8_t>(symbol), out);
                ++produced;
                state_ = state_ < 4 ? 0 : (state_ < 10 ? state_ - 3 : state_ - 6);
                continue;
            }
            std::uint32_t length = 0;
            if (range_.bit(isRep_[static_cast<std::size_t>(state_)]) == 0) {
                rep3_ = rep2_;
                rep2_ = rep1_;
                rep1_ = rep0_;
                length = decodeLength(matchLength_, posState);
                state_ = state_ < 7 ? 7 : 10;
                const std::uint32_t lengthState = std::min<std::uint32_t>(length, 3);
                const std::uint32_t slot = range_.tree(posSlot_[lengthState].data(), 6);
                if (slot < 4) {
                    rep0_ = slot;
                } else {
                    const int direct = static_cast<int>(slot >> 1) - 1;
                    std::uint32_t distance = (2 | (slot & 1)) << direct;
                    if (slot < 14) {
                        distance += range_.reverseTree(posSpecial_.data() + (distance - slot), direct);
                    } else {
                        distance += range_.direct(direct - 4) << 4;
                        distance += range_.reverseTree(align_.data(), 4);
                    }
                    rep0_ = distance;
                }
                if (rep0_ == 0xFFFFFFFFU) {
                    return false;
                }
            } else {
                if (range_.bit(isRepG0_[static_cast<std::size_t>(state_)]) == 0) {
                    if (range_.bit(isRep0Long_[static_cast<std::size_t>(state_)][posState]) == 0) {
                        state_ = state_ < 7 ? 9 : 11;
                        put(byteAt(rep0_ + 1), out);
                        ++produced;
                        continue;
                    }
                } else {
                    std::uint32_t distance = 0;
                    if (range_.bit(isRepG1_[static_cast<std::size_t>(state_)]) == 0) {
                        distance = rep1_;
                    } else {
                        if (range_.bit(isRepG2_[static_cast<std::size_t>(state_)]) == 0) {
                            distance = rep2_;
                        } else {
                            distance = rep3_;
                            rep3_ = rep2_;
                        }
                        rep2_ = rep1_;
                    }
                    rep1_ = rep0_;
                    rep0_ = distance;
                }
                length = decodeLength(repLength_, posState);
                state_ = state_ < 7 ? 8 : 11;
            }
            length += 2;
            if (rep0_ >= filled_ || length > unpacked - produced) {
                return false;
            }
            for (std::uint32_t i = 0; i < length; ++i) {
                put(byteAt(rep0_ + 1), out);
            }
            produced += length;
        }
        return !range_.overrun();
    }

private:
    std::uint32_t decodeLength(LengthCoder& coder, std::uint32_t posState) {
        if (range_.bit(coder.choice) == 0) {
            return range_.tree(coder.low[posState].data(), 3);
        }
        if (range_.bit(coder.choice2) == 0) {
            return 8 + range_.tree(coder.mid[posState].data(), 3);
        }
        return 16 + range_.tree(coder.high.data(), 8);
    }

    [[nodiscard]] std::uint8_t byteAt(std::size_t distance) const {
        return window_[(position_ + window_.size() - distance) % window_.size()];
    }

    void put(std::uint8_t value, std::vector<std::uint8_t>& out) {
        window_[position_] = value;
        position_ = (position_ + 1) % window_.size();
        filled_ = std::min(filled_ + 1, window_.size());
        ++totalPosition_;
        out.push_back(value);
    }

    RangeDecoder range_;
    std::vector<std::uint8_t> window_;
    std::size_t position_ = 0;
    std::size_t filled_ = 0;
    std::uint64_t totalPosition_ = 0;
    std::vector<std::uint16_t> literal_;
    std::array<std::array<std::uint16_t, kPosStates>, kStates> isMatch_{};
    std::array<std::array<std::uint16_t, kPosStates>, kStates> isRep0Long_{};
    std::array<std::uint16_t, kStates> isRep_{};
    std::array<std::uint16_t, kStates> isRepG0_{};
    std::array<std::uint16_t, kStates> isRepG1_{};
    std::array<std::uint16_t, kStates> isRepG2_{};
    std::array<std::array<std::uint16_t, 64>, 4> posSlot_{};
    std::array<std::uint16_t, 130> posSpecial_{};
    std::array<std::uint16_t, 16> align_{};
    LengthCoder matchLength_;
    LengthCoder repLength_;
    int lc_ = 0;
    int lp_ = 0;
    int pb_ = 0;
    int state_ = 0;
    std::uint32_t rep0_ = 0;
    std::uint32_t rep1_ = 0;
    std::uint32_t rep2_ = 0;
    std::uint32_t rep3_ = 0;
};

std::size_t checkSize(std::uint8_t type) {
    if (type == 0) {
        return 0;
    }
    if (type <= 3) {
        return 4;
    }
    if (type <= 6) {
        return 8;
    }
    if (type <= 9) {
        return 16;
    }
    return type <= 12 ? 32 : 64;
}

class XzSource final : public ByteSource {
public:
    explicit XzSource(const std::filesystem::path& path) : in_(path, std::ios::binary) {
        if (!in_) {
            error = "cannot read " + path.string();
            finished_ = true;
            return;
        }
        std::array<std::uint8_t, 12> header{};
        if (!exact(header.data(), header.size()) || std::memcmp(header.data(),
                                                                "\xFD"
                                                                "7zXZ\0",
                                                                6) != 0) {
            fail("not an xz stream");
            return;
        }
        checkSize_ = checkSize(header[7] & 0x0F);
    }

    std::size_t read(std::span<std::uint8_t> out) override {
        while (readPosition_ >= output_.size() && !finished_) {
            output_.clear();
            readPosition_ = 0;
            step();
        }
        const std::size_t count = std::min(out.size(), output_.size() - readPosition_);
        std::memcpy(out.data(), output_.data() + readPosition_, count);
        readPosition_ += count;
        return count;
    }

private:
    bool exact(std::uint8_t* data, std::size_t size) {
        in_.read(reinterpret_cast<char*>(data), static_cast<std::streamsize>(size));
        consumed_ += size;
        return static_cast<std::size_t>(in_.gcount()) == size;
    }

    void fail(const std::string& why) {
        error = "corrupt xz data: " + why;
        finished_ = true;
    }

    std::optional<std::uint64_t> varint(const std::vector<std::uint8_t>& data, std::size_t& at) {
        std::uint64_t value = 0;
        for (unsigned shift = 0; shift < 63 && at < data.size(); shift += 7) {
            const std::uint8_t next = data[at++];
            value |= static_cast<std::uint64_t>(next & 0x7F) << shift;
            if ((next & 0x80) == 0) {
                return value;
            }
        }
        return std::nullopt;
    }

    void startBlock(std::uint8_t sizeByte) {
        const std::size_t headerSize = (static_cast<std::size_t>(sizeByte) + 1) * 4;
        std::vector<std::uint8_t> header(headerSize);
        header[0] = sizeByte;
        if (!exact(header.data() + 1, headerSize - 1)) {
            fail("truncated block header");
            return;
        }
        std::size_t at = 1;
        const std::uint8_t flags = header[at++];
        if ((flags & 0x03) != 0) {
            fail("unsupported filter chain");
            return;
        }
        if ((flags & 0x40) != 0 && !varint(header, at).has_value()) {
            fail("bad block header");
            return;
        }
        if ((flags & 0x80) != 0 && !varint(header, at).has_value()) {
            fail("bad block header");
            return;
        }
        const auto filter = varint(header, at);
        const auto propertySize = varint(header, at);
        if (!filter.has_value() || *filter != 0x21 || !propertySize.has_value() || *propertySize != 1 ||
            at >= header.size()) {
            fail("the block does not use LZMA2");
            return;
        }
        const std::uint8_t dictionaryByte = header[at];
        if (dictionaryByte > 40) {
            fail("bad dictionary size");
            return;
        }
        const std::size_t dictionary =
            dictionaryByte == 40 ? kMaxDictionary
                                 : (static_cast<std::size_t>(2 | (dictionaryByte & 1)) << (dictionaryByte / 2 + 11));
        if (dictionary > kMaxDictionary) {
            fail("the dictionary is too large");
            return;
        }
        decoder_.setDictionary(dictionary);
        blockStart_ = consumed_ - headerSize;
        inBlock_ = true;
    }

    void step() {
        if (!inBlock_) {
            std::uint8_t first = 0;
            if (!exact(&first, 1)) {
                fail("truncated stream");
                return;
            }
            if (first == 0) {
                finished_ = true;
                return;
            }
            startBlock(first);
            return;
        }
        std::uint8_t control = 0;
        if (!exact(&control, 1)) {
            fail("truncated block");
            return;
        }
        if (control == 0) {
            const std::size_t used = consumed_ - blockStart_;
            std::vector<std::uint8_t> padding((4 - used % 4) % 4 + checkSize_);
            if (!padding.empty() && !exact(padding.data(), padding.size())) {
                fail("truncated block trailer");
                return;
            }
            inBlock_ = false;
            return;
        }
        if (control == 1 || control == 2) {
            std::array<std::uint8_t, 2> size{};
            if (!exact(size.data(), 2)) {
                fail("truncated chunk");
                return;
            }
            std::vector<std::uint8_t> raw((static_cast<std::size_t>(size[0]) << 8 | size[1]) + 1);
            if (!exact(raw.data(), raw.size())) {
                fail("truncated chunk");
                return;
            }
            if (control == 1) {
                decoder_.resetDictionary();
            }
            decoder_.appendRaw(raw.data(), raw.size(), output_);
            return;
        }
        if (control < 0x80) {
            fail("bad chunk header");
            return;
        }
        std::array<std::uint8_t, 4> sizes{};
        if (!exact(sizes.data(), 4)) {
            fail("truncated chunk");
            return;
        }
        const std::size_t unpacked =
            ((static_cast<std::size_t>(control & 0x1F) << 16) | (static_cast<std::size_t>(sizes[0]) << 8) | sizes[1]) +
            1;
        const std::size_t packed = ((static_cast<std::size_t>(sizes[2]) << 8) | sizes[3]) + 1;
        const int reset = (control >> 5) & 0x03;
        if (reset == 3) {
            decoder_.resetDictionary();
        }
        if (reset >= 2) {
            std::uint8_t properties = 0;
            if (!exact(&properties, 1) || !decoder_.setProperties(properties)) {
                fail("bad LZMA properties");
                return;
            }
        }
        if (reset >= 1) {
            decoder_.resetState();
        }
        std::vector<std::uint8_t> compressed(packed);
        if (!exact(compressed.data(), packed)) {
            fail("truncated chunk");
            return;
        }
        if (!decoder_.decode(compressed.data(), compressed.size(), unpacked, output_)) {
            fail("LZMA data does not decode");
        }
    }

    std::ifstream in_;
    Lzma2Decoder decoder_;
    std::vector<std::uint8_t> output_;
    std::size_t readPosition_ = 0;
    std::size_t checkSize_ = 0;
    std::uint64_t consumed_ = 0;
    std::uint64_t blockStart_ = 0;
    bool inBlock_ = false;
    bool finished_ = false;
};

} // namespace

std::unique_ptr<ByteSource> openXzSource(const std::filesystem::path& path) { return std::make_unique<XzSource>(path); }

} // namespace basalt::kiln
