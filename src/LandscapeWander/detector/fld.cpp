#if true
#include <opencv2/ximgproc/fast_line_detector.hpp>
#include "../../Utility/numeric.hpp"
#include "../CVUtility.hpp"
#include "fld.hpp"

namespace LandscapeWander::detector {

static constexpr double base_param = 0.50;

static Array<Line> obtain_lines_from_pictures_fld(
    const Image& img,
    const double alpha,
    const double beta
);

/**
 * @brief 
 * 
 * @param image 
 * @param alpha 
 * @param beta 
 * @param gamma 
 * @return Array<Line> 
 */
Array<Line> fld(
    const Image& image,
    const double alpha,
    const double beta,
    const double gamma
) {
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

    // 特徴線を抽出する。
    Array<Line> lines = obtain_lines_from_pictures_fld(image_scaled, alpha, beta);
    Console << U"detected lines by fld: " << lines.size();
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

static Array<Line> obtain_lines_from_pictures_fld(
    const Image& img,
    const double alpha,
    const double beta
) {
    assert(0 <= alpha and alpha <= 1);
    assert(0 <= beta and beta <= 1);

    const int32_t height = img.height();
    const float canny_min = lerp(alpha, 20, base_param) + 10;
    const float canny_max = lerp(alpha, 70, base_param) + 30;
    const double line_length   = lerp(1-beta, 7, base_param) + 3;
    const double distance      = lerp(1-beta, 1.314, base_param) + 0.1;
    
    cv::Ptr<cv::ximgproc::FastLineDetector> fld = cv::ximgproc::createFastLineDetector(
        line_length, // length th
        distance, // distance th
        canny_min, canny_max, // canny_th
        3, // canny_aparture
        true // do_marge
    );
    cv::Mat img_grayed_mat = image_to_gray_mat(img);

    std::vector<cv::Vec4i> lines_from_pictures;
    fld->detect(img_grayed_mat, lines_from_pictures);
    
    return vectorCvVec4_to_arrayLine(lines_from_pictures);
}
}

#endif