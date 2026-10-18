#pragma once

#include "base_filters.h"
#include "parse_args.h"
#include <vector>

std::unique_ptr<Filter> CreateFilter(const FilterSpec& spec);