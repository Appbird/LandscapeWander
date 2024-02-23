#include <opencv2/opencv.hpp>
#include <algorithm>
#include <iterator>
#include "image_process.hpp"
 
static double segment_distance(double l1, double r1, double l2, double r2) { 
    if (r1 < l1) { return segment_distance(r1, l1, l2, r2); }
    if (r2 < l2) { return segment_distance(l1, r1, r2, l2); }
    return std::max(0.0, std::max(l1, l2) - std::min(r2, r1));
}

static cv::Mat image_to_gray_mat(const Image& image_scaled) {
    Image img_gray = image_scaled.grayscaled();

    // そのためのCanny, Hough変換を利用するべくいったんOpenCVのMatに変換する。
    cv::Mat img_grayed_mat = cv::Mat::zeros(img_gray.height(), img_gray.width(), CV_8U);
    for (int r = 0; r < img_gray.height(); r++){
        for (int c = 0; c < img_gray.width(); c++) {
            img_grayed_mat.at<uint8_t>(r, c) = img_gray[r][c].r;
        }
    }
    return img_grayed_mat;
}

static Array<Line> vectorCvVec4_to_arrayLine(const std::vector<cv::Vec4i>& lines_from_pictures) {
    const uint64_t line_count = lines_from_pictures.size();
    Array<Line> lines; lines.reserve(line_count);
    for (const cv::Vec4i& line:lines_from_pictures){
        lines.push_back({
            {line[0], line[1]}, {line[2], line[3]}
        });
    }
    return lines;
}

static Array<Line> obtain_lines_from_pictures(
    const Image& img,
    const double alpha
) {
    constexpr float canny_min = 30;
    constexpr float canny_max = 100; 
    constexpr float elected_coef = 0.01;
    constexpr double min_line_length_coef   = 1.0/30;
    constexpr double max_line_gap_coef      = 1.0/15;

    cv::Mat img_grayed_mat = image_to_gray_mat(img);
    cv::Mat cannyed;
    cv::Canny(img_grayed_mat, cannyed, canny_min, canny_max);
    const int features_point_count = cv::countNonZero(cannyed);

    std::vector<cv::Vec4i> lines_from_pictures;
    const auto threshold = int32_t(features_point_count * elected_coef);
    cv::HoughLinesP(
        cannyed, lines_from_pictures,
        1, M_PI/180,
        threshold,
        img.width() * min_line_length_coef,
        img.width() * max_line_gap_coef
    );
    return vectorCvVec4_to_arrayLine(lines_from_pictures);
}

static Array<Line> combine_lines(const Array<Line>& given_lines, const double image_width, const double alpha) {
    constexpr double angle_threshold = M_PI / 12;
    constexpr double transverse_threshold = 10;
    constexpr double longitudinal_threshold = 20;

    Array<Line> lines = given_lines;
    // line_used[i] == true: まだ線iが統合されていない
    // OpenCVで得られたデータをLine型に変換する。
    const size_t line_count = lines.size();
    Array<bool> line_used(line_count, true);
    // 各辺の組み合わせのうち、最も近くて角度が類似している辺を結合する。
    for (size_t i = 0; i < line_count; i++) {
        for (size_t j = i + 1; j < line_count; j++) {
            if (not line_used[i] or not line_used[j]) { continue; }
            
            const Line& L1 = lines[i];
            const Line& L2 = lines[j];
            // 傾きが十分に似ている2線分だけに注目する。
            // 差が0か2πに近い2線分だけに着目する。
            if (
                M_PI - abs(M_PI - abs(L1.vector().getAngle() - L2.vector().getAngle())) > angle_threshold
            ) { continue; }
            // L2の端点をL1の直線上に移動させる。
            // この地点で移動距離が十分に小さいもののみを統合対象とする。
            const Vec2 L1_direction = L1.vector().normalized();
            // L2をL1へ統合した後の位置。ただし、L1とL2の距離が十分に遠ければ、それらは統合しない。
            const Vec2 L2b_prime = (L2.begin - L1.begin).dot(L1_direction) * L1_direction + L1.begin;
            const Vec2 L2e_prime = (L2.end - L1.begin).dot(L1_direction) * L1_direction + L1.begin;
            
            // 横方向に離れすぎていない線分のみを結合する。
            const double& x = transverse_threshold;
            if (
                (L2b_prime - L2.begin).lengthSq() > x*x or (L2e_prime - L2.end).lengthSq() > x*x
            ) {
                continue;
            }
            // 縦方向に離れすぎていない線分のみを結合する。
            const auto projection = [&](const Vec2& v) -> double { return (v - L1.begin).dot(L1_direction); };
            const double& y = longitudinal_threshold;
            Console << segment_distance(
                    projection(L1.begin), projection(L1.end),
                    projection(L2.begin), projection(L2.end)
                );
            if (
                segment_distance(
                    projection(L1.begin), projection(L1.end),
                    projection(L2.begin), projection(L2.end)
                ) > y
            ) {
                continue;
            }

            // #COMPLETED TODO projectionから書く
            // 線分の構成点を、すべてのL1方向の軸へ降ろしたときに各々の点がどのような順序で並ぶかを調べる。
            // その並びをもとに、端にある端点を調べる。
            std::array<Vec2, 4> point_list{L1.begin, L1.end, L2b_prime, L2e_prime};
            
            std::sort(point_list.begin(), point_list.end(), 
                [&](const Vec2& a, const Vec2& b) -> bool { return projection(a) < projection(b); }
            );
            const Vec2& min_point = point_list[0];
            const Vec2& max_point = point_list[3];
            
            lines[i] = {min_point, max_point};
            line_used[j] = false;
        }
    }

    // 統合されていない線のみを抽出し
    Array<Line> tied_lines;
    for (size_t i = 0; i < line_count; i++) {
        if (line_used[i]) { tied_lines.push_back(lines[i]); }
    }
    return tied_lines;
}

Array<Line> extract_stageline_from(const Image& image, const double alpha) {
    constexpr int N = 5;
    constexpr int d = 10;
    constexpr double sigma_color = 90;
    constexpr double sigma_space = 90;

    // 処理負荷の軽減のため、画像を小さくする
    constexpr int after_height = 300;
    const double f = double(after_height) / image.height();
    Image image_scaled = image.scaled(f);

    // 輪郭線以外の情報をなるべく消す。
    for (int i = 0; i < N; i++) {
        image_scaled.bilateralFilter(d, sigma_color, sigma_space);
    }
    // # 特徴線を抽出する。
    Array<Line> lines = obtain_lines_from_pictures(image_scaled, alpha);
    Console << U"tied_lines: " << lines.size();
    Array<Line> tied_lines = combine_lines(lines, image_scaled.width(), alpha);
    Console << U"tied_lines: " << tied_lines.size();
    
    Rect image_region{{0, 0}, {image_scaled.width(), image_scaled.height()}};
    // 底に線を追加
    tied_lines.emplace_back(image_region.bottom());


    // もともと画像サイズを圧縮していたので、得られた線分の座標を画像座標系のそれに戻す。
    for (Line& line : tied_lines) {
        line.begin /= f;
        line.end /= f;
    }
    return tied_lines;
}