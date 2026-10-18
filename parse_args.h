#pragma once

#include "errors.h"
#include <string_view>
#include <vector>
#include <charconv>

template <typename T>
T Parse(std::string_view text) {
    if (text.empty()) {
        throw ERROR::InvalidArgument("Expected number");
    }
    T value = 0;
    if constexpr (std::is_same_v<T, double>) {
        const std::string str(text);
        size_t pos = 0;
        try {
            value = std::stod(str, &pos);
        } catch (const std::out_of_range) {
            throw ERROR::OutOfRange("Number is out of range");
        };
        if (pos != str.size()) {
            throw ERROR::InvalidArgument("Unexpected characters after number");
        }
        if (!std::isfinite(value)) {
            throw ERROR::InvalidArgument("Expected finite number");
        }
    } else {
        const auto [ptr, err] = std::from_chars(text.data(), text.data() + text.size(), value);
        if (err == std::errc::result_out_of_range) {
            throw ERROR::OutOfRange("Number is out of range");
        }
        if (err != std::errc{} || ptr != text.data() + text.size()) {
            throw ERROR::InvalidArgument("Invalid number");
        }
    }
    return value;
}

struct FilterSpec {
    std::string_view name;
    std::vector<std::string_view> params;
};

struct Args {
    std::string_view input;
    std::string_view output;
    std::vector<FilterSpec> filters;
};

void CheckFilterCorrectness(const FilterSpec& filter);

Args PargeArgs(const std::vector<std::string_view>& params);