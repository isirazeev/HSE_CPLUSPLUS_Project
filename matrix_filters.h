#pragma once

#include "errors.h"
#include "parse_args.h"
#include "base_filters.h"
#include "pixel_filters.h"

class SharpeningFilter : public MatrixFilter {
public:
    explicit SharpeningFilter(const FilterSpec& spec)
        : Filter(spec), MatrixFilter(spec, {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}}) {};
};

class EdgeDetectionFilter : public GrayScaleFilter, public MatrixFilter {
public:
    explicit EdgeDetectionFilter(const FilterSpec& spec)
        : Filter(spec), GrayScaleFilter(spec), MatrixFilter(spec, {{0, -1, 0}, {-1, 4, -1}, {0, -1, 0}}) {
        threshold = Parse<double>(spec.params[0]);
        if (!std::isfinite(threshold) || threshold < 0.0 || threshold > 1.0) {
            throw ERROR::IncorrectFilter(std::string(spec.name) + ":argument is out of range [0, 1]");
        }
    }

    void Transform(Image& image) override {
        GrayScaleFilter::Transform(image);
        MatrixFilter::Transform(image);

        for (size_t x = 0; x < image.Width(); ++x) {
            for (size_t y = 0; y < image.Height(); ++y) {
                if (image.At(x, y).r > threshold) {
                    image.At(x, y) = Color(1, 1, 1);
                } else {
                    image.At(x, y) = Color(0, 0, 0);
                }
            }
        }
    }

    void Apply(Image& image) override {
        Transform(image);
    }

private:
    double threshold;
};