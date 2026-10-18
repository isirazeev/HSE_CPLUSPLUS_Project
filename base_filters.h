#pragma once

#include "errors.h"
#include "parse_args.h"
#include "image.h"

namespace {
const double INF = 1e18;
const double sq = std::sqrt(3);
}  // namespace

class Filter {
public:
    explicit Filter(const FilterSpec &spec) : params(spec) {};

    virtual void Apply(Image &image) = 0;

    virtual ~Filter() = default;

    FilterSpec params;
};

class PixelFilter : public virtual Filter {
public:
    explicit PixelFilter(const FilterSpec &spec) : Filter(spec) {};

    virtual Color PixelOperation(const Color &pixel) {
        return Color(pixel);
    }

    virtual void Transform(Image &image) {
        for (size_t i = 0; i < image.Width(); ++i) {
            for (size_t j = 0; j < image.Height(); ++j) {
                image.At(i, j) = PixelOperation(image.At(i, j));
            }
        }
    }

    void Apply(Image &image) override {
        Transform(image);
    }
};

class MatrixFilter : public virtual Filter {
public:
    explicit MatrixFilter(const FilterSpec &spec, std::vector<std::vector<int>> mat_) : Filter(spec), mat(mat_) {};

    virtual void Transform(Image &image) {
        Image result(image.Width(), image.Height());
        for (size_t x = 0; x < image.Width(); ++x) {
            for (size_t y = 0; y < image.Height(); ++y) {
                result.At(x, y) = ColorOperations::min(
                    Color(1, 1, 1),
                    ColorOperations::max(Color(0, 0, 0),
                                         image.At(x - 1, y - 1) * mat[0][0] + image.At(x, y - 1) * mat[0][1] +
                                             image.At(x + 1, y - 1) * mat[0][2] + image.At(x - 1, y) * mat[1][0] +
                                             image.At(x, y) * mat[1][1] + image.At(x + 1, y) * mat[1][2] +
                                             image.At(x - 1, y + 1) * mat[2][0] + image.At(x, y + 1) * mat[2][1] +
                                             image.At(x + 1, y + 1) * mat[2][2]));
            }
        }
        image = std::move(result);
    }

    void Apply(Image &image) override {
        Transform(image);
    }

private:
    std::vector<std::vector<int>> mat;
};

class CropFilter : public Filter {
public:
    explicit CropFilter(const FilterSpec &spec) : Filter(spec) {
        width = Parse<size_t>(spec.params[0]);
        height = Parse<size_t>(spec.params[1]);
        if (width <= 0 || height <= 0) {
            throw ERROR::IncorrectFilter(std::string(spec.name) + " :Invalid arguments");
        }
    };
    void Apply(Image &image) override {
        Image result(std::min(width, image.Width()), std::min(height, image.Height()));
        for (size_t x = 0; x < std::min(width, image.Width()); ++x) {
            for (size_t y = 0; y < std::min(height, image.Height()); ++y) {
                result.At(x, y) = image.At(x, y);
            }
        }
        image = std::move(result);
    }

private:
    size_t width;
    size_t height;
};

class HexFilter : public Filter {
public:
    explicit HexFilter(const FilterSpec &spec) : Filter(spec) {
        cX = Parse<double>(spec.params[0]);
        cY = Parse<double>(spec.params[1]);
        r = Parse<double>(spec.params[2]);

        if (r < 1.0 || r > 1000.0) {
            throw ERROR::IncorrectFilter(std::string(spec.name) + " :radius must be in range [1, 1000]");
        }
    }

    std::pair<double, double> FindNearestCenter(double x, double y) {
        const double step_x = sq * r;
        const double step_y = 1.5 * r;

        double distance = INF;
        std::pair<double, double> result;

        const int nearest_row = std::round((y - cY) / step_y);
        for (int row = nearest_row - 1; row <= nearest_row + 1; ++row) {
            double add_shift = 0.0;
            if (row % 2 != 0) {
                add_shift = 0.5;
            }
            int col = std::round((x - cX) / step_x - add_shift);

            double nX = cX + step_x * (col + add_shift);
            double nY = cY + step_y * row;

            double dist = (nX - x) * (nX - x) + (nY - y) * (nY - y);
            if (dist < distance) {
                distance = dist;
                result = {nX, nY};
            }
        }

        return result;
    }

    void Apply(Image &image) override {
        const double max_x = static_cast<double>(image.Width()) - 1.0;
        const double max_y = static_cast<double>(image.Height()) - 1.0;

        if (cX < 0.0 || cX > max_x || cY < 0.0 || cY > max_y) {
            throw ERROR::IncorrectFilter("hex :center must be inside the image");
        }

        Image result(image.Width(), image.Height());
        for (size_t x = 0; x < image.Width(); ++x) {
            for (size_t y = 0; y < image.Height(); ++y) {
                auto [nX, nY] = FindNearestCenter(x, y);
                result.At(x, y) = image.At(static_cast<int>(std::clamp(std::round(nX), 0.0, max_x)),
                                           static_cast<int>(std::clamp(std::round(nY), 0.0, max_y)));
            }
        }
        image = std::move(result);
    }

private:
    double cX;
    double cY;
    double r;
};