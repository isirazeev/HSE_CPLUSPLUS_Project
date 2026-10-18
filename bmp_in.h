#pragma once

#include "image.h"
#include <cstdint>
#include <fstream>

namespace ImageRead {

namespace {
constexpr uint16_t kSignature = 0x4D42;
constexpr uint16_t kReserved = 0;
constexpr uint32_t kDibSize = 40;
constexpr uint16_t kPlanes = 1;
constexpr uint16_t kBitCount = 24;
constexpr uint32_t kCompression = 0;
constexpr uint32_t kClrUsed = 0;
constexpr uint32_t kHeaderSize = 54;
};  // namespace

template <typename T>
T ReadTo(std::istream &in) {
    unsigned char bytes[sizeof(T)];
    if (!in.read(reinterpret_cast<char *>(bytes), sizeof(T))) {
        throw ERROR::BMPError("BMP file damaged");
    }
    uint64_t value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value |= static_cast<uint64_t>(bytes[i]) << (8 * i);
    }
    return static_cast<T>(value);
}

Image Read(std::string_view path) {
    std::ifstream in(std::string(path), std::ios::binary);
    if (!in) {
        throw ERROR::BMPError("Cannot open BMP file: " + std::string(path));
    }
    const auto signature = ReadTo<uint16_t>(in);
    const auto file_size = ReadTo<uint32_t>(in);
    const auto reserved1 = ReadTo<uint16_t>(in);
    const auto reserved2 = ReadTo<uint16_t>(in);
    const auto pixel_offset = ReadTo<uint32_t>(in);

    if (signature != kSignature) {
        throw ERROR::BMPError("Invalid BMP signature");
    }
    if (reserved1 != kReserved || reserved2 != kReserved) {
        throw ERROR::BMPError("Invalid BMP reserved fields");
    }

    const auto dib_size = ReadTo<uint32_t>(in);

    if (dib_size != kDibSize) {
        throw ERROR::BMPError("Expected BITMAPINFOHEADER");
    }

    const auto width = ReadTo<int32_t>(in);
    const auto signed_height = ReadTo<int32_t>(in);
    const auto planes = ReadTo<uint16_t>(in);
    const auto bit_count = ReadTo<uint16_t>(in);
    const auto compression = ReadTo<uint32_t>(in);
    const auto image_size = ReadTo<uint32_t>(in);
    const auto x_pixels_per_meter = ReadTo<uint32_t>(in);
    const auto y_pixels_per_meter = ReadTo<uint32_t>(in);
    const auto colors_used = ReadTo<uint32_t>(in);
    const auto colors_important = ReadTo<uint32_t>(in);

    if (planes != kPlanes || bit_count != kBitCount || compression != kCompression || colors_used != kClrUsed) {
        throw ERROR::BMPError("Expected uncompressed 24-bit BMP without a color table");
    }
    if (width <= 0 || signed_height == 0) {
        throw ERROR::BMPError("Invalid BMP dimensions");
    }
    int64_t height;
    if (signed_height < 0) {
        height = -static_cast<int64_t>(signed_height);
    } else {
        height = signed_height;
    }

    if (height > std::numeric_limits<int>::max()) {
        throw ERROR::BMPError("BMP height is too large");
    }

    const uint64_t row_bytes = 3ull * width;
    const uint64_t row_size = (row_bytes + 3) / 4 * 4;

    const uint64_t data_size = row_size * height;
    if (pixel_offset < kHeaderSize || file_size < data_size + pixel_offset) {
        throw ERROR::BMPError("Invalid BMP pixel data size or offset");
    }
    if (image_size != 0 && image_size != data_size) {
        throw ERROR::BMPError("Invalid BMP image size");
    }
    in.seekg(0, std::ios::end);
    const auto sz = in.tellg();
    if (sz == std::streampos(-1) || static_cast<uint64_t>(static_cast<std::streamoff>(sz)) != file_size) {
        throw ERROR::BMPError("BMP file size does not match its header");
    }

    in.seekg(pixel_offset, std::ios::beg);
    if (!in) {
        throw ERROR::BMPError("Cannot go to BMP pixel data");
    }

    const auto image_width = static_cast<size_t>(width);
    const auto image_height = static_cast<size_t>(height);

    Image result(image_width, image_height);
    std::vector<unsigned char> row(row_size);
    for (int j = 0; j < static_cast<int>(image_height); ++j) {
        if (!in.read(reinterpret_cast<char *>(row.data()), row_size)) {
            throw ERROR::BMPError("BMP pixel data damaged");
        }
        int y;
        if (signed_height > 0) {
            y = height - 1 - j;
        } else {
            y = j;
        }
        for (int x = 0; x < static_cast<int>(image_width); ++x) {
            const auto ind = static_cast<size_t>(x) * 3;
            result.At(x, y) = Color(row[ind + 2] / 255.0, row[ind + 1] / 255.0, row[ind] / 255.0);
        }
    }
    return result;
}

}  // namespace ImageRead
