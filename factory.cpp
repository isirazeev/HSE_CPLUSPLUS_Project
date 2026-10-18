#include "factory.h"
#include "base_filters.h"
#include "pixel_filters.h"
#include "matrix_filters.h"

#include <unordered_map>

namespace {
using Creator = std::unique_ptr<Filter> (*)(const FilterSpec &spec);

template <typename T>
std::unique_ptr<Filter> Make(const FilterSpec &spec) {
    return std::make_unique<T>(spec);
}

const std::unordered_map<std::string_view, Creator> kCreators = {{"crop", Make<CropFilter>},
                                                                 {"gs", Make<GrayScaleFilter>},
                                                                 {"neg", Make<NegFilter>},
                                                                 {"blur", Make<GaussianBlurFilter>},
                                                                 {"edge", Make<EdgeDetectionFilter>},
                                                                 {"sharp", Make<SharpeningFilter>},
                                                                 {"hex", Make<HexFilter>}};
};  // namespace

std::unique_ptr<Filter> CreateFilter(const FilterSpec &spec) {
    const auto it = kCreators.find(spec.name);
    if (it == kCreators.end()) {
        throw ERROR::IncorrectFilter("Unknown filter");
    }
    return it->second(spec);
}