#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <algorithm>
#include <iterator>
#include "../../Utility/numeric.hpp"
#include "../CVUtility.hpp"
#include "houghp.hpp" 

namespace LandscapeWander::detector {

static constexpr double base_param = 0.50;

static Array<Line> obtain_lines_from_pictures(
    const Image& img,
    const double alpha,
    const double beta
);




Array<Line> houghp(const Image& image, const double alpha, const double beta, const double gamma) {
    assert(0 <= alpha and alpha <= 1);
    assert(0 <= beta and beta <= 1);
    assert(0 <= gamma and gamma <= 1);
    constexpr int N = 3;
    constexpr int d = 8;
    const double sigma_color = lerp(alpha, 90, base_param);
    const double sigma_space = lerp(alpha, 90, base_param);

    // 処理負荷の軽減のため、画像を小さくする
    const int after_height = 600;
    const double f = double(after_height) / image.height();
    Image image_scaled = image.scaled(f);

    // 輪郭線以外の情報をなるべく消す。
    for (int i = 0; i < N; i++) {
        image_scaled.bilateralFilter(d, sigma_color, sigma_space);
    }

    // # 特徴線を抽出する。
    Array<Line> lines = obtain_lines_from_pictures(image_scaled, alpha, beta);
    Console << U"detected lines: " << lines.size();
    Array<Line> tied_lines = combine_lines(lines, after_height, gamma, base_param);
    Console << U"tied_lines: " << tied_lines.size();
    
    Rect image_region{{0, 0}, {image_scaled.width(), image_scaled.height()-1}};
    // 底に線を追加
    tied_lines.emplace_back(image_region.bottom());


    // もともと画像サイズを圧縮していたので、得られた線分の座標を画像座標系のそれに戻す。
    for (Line& line : tied_lines) {
        line.begin /= f;
        line.end /= f;
    }
    
    return tied_lines;
}

static Array<Line> obtain_lines_from_pictures(
    const Image& img,
    const double alpha,
    const double beta
) {
    assert(0 <= alpha and alpha <= 1);
    assert(0 <= beta and beta <= 1);

    const int32_t height = img.height();
    const float canny_min = lerp(alpha, 29, base_param) + 1;
    const float canny_max = lerp(alpha, 99, base_param) + 1; 
    const double min_line_length   = height * lerp(1-beta, 1.0/10, base_param);
    const double max_line_gap      = height * lerp(1-beta, 1.0/20, base_param);
    
    cv::Mat img_grayed_mat = image_to_gray_mat(img);
    cv::Mat cannyed;
    cv::Canny(img_grayed_mat, cannyed, canny_min, canny_max);

    std::vector<cv::Vec4i> lines_from_pictures;
    const auto threshold = height / 10;
    cv::HoughLinesP(
        cannyed, lines_from_pictures,
        1, M_PI/180,
        threshold, min_line_length, max_line_gap
    );
    
    return vectorCvVec4_to_arrayLine(lines_from_pictures);
}

}