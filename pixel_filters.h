#pragma once

#include "errors.h"
#include "parse_args.h"
#include "image.h"
#include "base_filters.h"
#include <numbers>

namespace {
const double kMaxSigma = 1000;
const double PI = std::numbers::pi;
};  // namespace

class GrayScaleFilter : public PixelFilter {
public:
    explicit GrayScaleFilter(const FilterSpec &spec) : Filter(spec), PixelFilter(spec) {};

    Color PixelOperation(const Color &pixel) override {
        return Color(pixel.r * 0.299 + pixel.g * 0.587 + pixel.b * 0.114,
                     pixel.r * 0.299 + pixel.g * 0.587 + pixel.b * 0.114,
                     pixel.r * 0.299 + pixel.g * 0.587 + pixel.b * 0.114);
    }
};

class NegFilter : public PixelFilter {
public:
    explicit NegFilter(const FilterSpec &spec) : Filter(spec), PixelFilter(spec) {};

    Color PixelOperation(const Color &pixel) override {
        return Color(1 - pixel.r, 1 - pixel.g, 1 - pixel.b);
    }
};

class GaussianBlurFilter : public PixelFilter {
public:
    explicit GaussianBlurFilter(const FilterSpec &spec) : Filter(spec), PixelFilter(spec) {
        sigma = Parse<double>(spec.params[0]);
        if (!std::isfinite(sigma)) {
            throw ERROR::IncorrectFilter(std::string(spec.name) + ":Invalid argument");
        }
        if (sigma <= 0) {
            throw ERROR::IncorrectFilter(std::string(spec.name) + ":Value must be positive");
        }
        if (sigma > kMaxSigma) {
            throw ERROR::IncorrectFilter(std::string(spec.name) + ":Value is too large");
        }
    }

    void Transform(Image &image) override {
        Image result(image.Width(), image.Height());
        const int radius = ceil(3 * sigma);
        const double factor = 1 / (2 * PI * sigma * sigma);
        for (size_t x0 = 0; x0 < image.Width(); x0++) {
            for (size_t y0 = 0; y0 < image.Height(); y0++) {
                Color sum(0, 0, 0);
                for (int offset_x = -radius; offset_x <= radius; ++offset_x) {
                    const int x = x0 + offset_x;
                    if (x < 0 || x >= static_cast<int>(image.Width())) {
                        continue;
                    }
                    for (int offset_y = -radius; offset_y <= radius; ++offset_y) {
                        const int y = y0 + offset_y;
                        if (y < 0 || y >= static_cast<int>(image.Height())) {
                            continue;
                        }
                        const int dist = offset_x * offset_x + offset_y * offset_y;
                        sum = sum + image.At(x, y) * factor * std::exp(-dist / (2 * sigma * sigma));
                    }
                }
                result.At(x0, y0) = sum;
            }
        }
        image = std::move(result);
    }

private:
    double sigma;
};