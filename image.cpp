#include "image.h"

Color ColorOperations::min(Color a, Color b) {
    return Color(std::min(a.r, b.r), std::min(a.g, b.g), std::min(a.b, b.b));
}

Color ColorOperations::max(Color a, Color b) {
    return Color(std::max(a.r, b.r), std::max(a.g, b.g), std::max(a.b, b.b));
}