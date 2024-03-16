#pragma once
#include <Siv3D.hpp>

Array<Line> extract_stageline_from(
    const Image& image,
    const double alpha = 0.5,
    const double beta = 0.5,
    const double gamma = 0.5
);