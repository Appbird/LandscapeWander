#pragma once
#include <Siv3D.hpp>
#include <opencv2/opencv.hpp>

cv::Mat image_to_gray_mat(const Image& image_scaled);
Array<Line> vectorCvVec4_to_arrayLine(const std::vector<cv::Vec4i>& lines_from_pictures);
void save_line_detection(const Image& image, const Array<Line>& lines);
Array<Line> combine_lines(
    const Array<Line>& given_lines,
    const int32_t image_height,
    const double gamma,
    const double base_param
);