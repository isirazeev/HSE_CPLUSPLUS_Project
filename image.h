#pragma once

#include <vector>

struct Color {
    double r = 0;
    double g = 0;
    double b = 0;

    Color() = default;

    Color(double r_, double g_, double b_) : r(r_), g(g_), b(b_) {};

    Color operator+(Color other) const {
        return Color(r + other.r, g + other.g, b + other.b);
    }

    Color operator*(double k) const {
        return Color(k * r, k * g, k * b);
    }
};

namespace ColorOperations {

Color min(Color a, Color b);

Color max(Color a, Color b);

}  // namespace ColorOperations

class Image {
public:
    Image(const size_t &width_, const size_t &height_) {
        width = width_;
        height = height_;
        pixels.resize(width_ * height_);
    }

    size_t Width() const {
        return width;
    }

    size_t Height() const {
        return height;
    }

    Color &At(const int &x, const int &y) {
        return pixels[std::clamp(y, 0, static_cast<int>(height - 1)) * width +
                      std::clamp(x, 0, static_cast<int>(width - 1))];
    }

    const Color &At(const int &x, const int &y) const {
        return pixels[std::clamp(y, 0, static_cast<int>(height - 1)) * width +
                      std::clamp(x, 0, static_cast<int>(width - 1))];
    }

private:
    size_t width;
    size_t height;
    std::vector<Color> pixels;
};