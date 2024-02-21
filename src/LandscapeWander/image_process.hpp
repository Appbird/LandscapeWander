#pragma once
#include <Siv3D.hpp>

Array<Line> extract_stageline_from(const Image& image);
Array<Line> extract_stageline_from_with_tmp_result(const Image& image, const double alpha);