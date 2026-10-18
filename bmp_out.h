#pragma once

#include "image.h"
#include <cstdint>
#include <fstream>

namespace ImageWrite {

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
void WriteTo(T value, std::ostream &out) {
    const uint64_t bits = static_cast<uint64_t>(value);
    unsigned char bytes[sizeof(T)];
    for (size_t i = 0; i < sizeof(T); ++i) {
        bytes[i] = static_cast<unsigned char>((bits >> (8 * i)) & 0xFF);
    }
    if (!out.write(reinterpret_cast<char *>(bytes), sizeof(T))) {
        throw ERROR::FileError("Cannot write BMP data");
    }
}

unsigned char Convert(double x) {
    return static_cast<unsigned char>(std::lround(x * 255.0));
}

void Write(const Image &image, std::string_view path) {
    std::ofstream out(std::string(path), std::ios::binary);

    const int32_t image_width = image.Width();
    const int32_t image_height = image.Height();
    const uint64_t row_bytes = 3ull * image_width;
    const uint64_t row_size = (row_bytes + 3) / 4 * 4;
    const uint32_t data_size = row_size * image_height;
    const uint32_t file_size = kHeaderSize + data_size;
    WriteTo<uint16_t>(kSignature, out);
    WriteTo<uint32_t>(file_size, out);
    WriteTo<uint16_t>(kReserved, out);
    WriteTo<uint16_t>(kReserved, out);
    WriteTo<uint32_t>(kHeaderSize, out);
    WriteTo<uint32_t>(kDibSize, out);
    WriteTo<int32_t>(image_width, out);
    WriteTo<int32_t>(image_height, out);
    WriteTo<uint16_t>(kPlanes, out);
    WriteTo<uint16_t>(kBitCount, out);
    WriteTo<uint32_t>(kCompression, out);
    WriteTo<uint32_t>(data_size, out);
    WriteTo<int32_t>(0, out);
    WriteTo<int32_t>(0, out);
    WriteTo<uint32_t>(kClrUsed, out);
    WriteTo<uint32_t>(0, out);
    std::vector<unsigned char> row(static_cast<size_t>(row_size), 0);

    for (int file_y = 0; file_y < image_height; ++file_y) {
        const int y = image_height - 1 - file_y;

        for (int x = 0; x < image_width; ++x) {
            const Color &pixel = image.At(x, y);
            const size_t offset = static_cast<size_t>(x) * 3;
            row[offset] = Convert(pixel.b);
            row[offset + 1] = Convert(pixel.g);
            row[offset + 2] = Convert(pixel.r);
        }
        if (!out.write(reinterpret_cast<char *>(row.data()), row_size)) {
            throw ERROR::FileError("Cannot write BMP data");
        }
    }

    out.close();
    if (!out) {
        throw ERROR::FileError("Cannot finish writing BMP file");
    }
}

}  // namespace ImageWrite
