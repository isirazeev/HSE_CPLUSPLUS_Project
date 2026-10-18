#include "parse_args.h"
#include "errors.h"

#include <unordered_map>

std::unordered_map<std::string_view, size_t> filter_params_counts = {{"crop", 2}, {"gs", 0},   {"neg", 0}, {"sharp", 0},
                                                                     {"edge", 1}, {"blur", 1}, {"hex", 3}};

void CheckFilterCorrectness(const FilterSpec& filter) {
    if (filter_params_counts.find(filter.name) == filter_params_counts.end()) {
        throw ERROR::IncorrectFilter("Unknown filter name: " + std::string(filter.name));
    }
    if (filter.params.size() != filter_params_counts[filter.name]) {
        throw ERROR::IncorrectFilter("Expecter another number of arguments for filter: " + std::string(filter.name));
    }
}

Args PargeArgs(const std::vector<std::string_view>& params) {
    Args result;
    if (params.size() < 2) {
        throw ERROR::FileError("No input or output file specified");
    }
    result.input = params[0];
    result.output = params[1];
    for (size_t i = 2; i < params.size(); ++i) {
        if (!params[i].starts_with("-")) {
            throw ERROR::IncorrectFilter("Filter should begin with -");
        }
        FilterSpec current_filter;
        current_filter.name = params[i].substr(1);
        size_t j = i + 1;
        while (j < params.size() && !params[j].starts_with("-")) {
            current_filter.params.push_back(params[j]);
            ++j;
        }
        i = j - 1;
        CheckFilterCorrectness(current_filter);
        result.filters.push_back(current_filter);
    }
    return result;
}