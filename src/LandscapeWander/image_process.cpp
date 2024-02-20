#include <opencv2/opencv.hpp>
#include <algorithm>
#include <iterator>
#include "image_process.hpp"
 
static double segment_distance(double l1, double r1, double l2, double r2) { 
    return std::max(0.0, std::max(l1, l2) - std::min(r2, r1));
}

Array<Line> extract_stageline_from(const Image& image) {
    constexpr int N = 5;
    constexpr int d = 10;
    constexpr double sigma_color = 90;
    constexpr double sigma_space = 90;

    constexpr float canny_min = 30;
    constexpr float canny_max = 100; 

    constexpr float elected_coef = 0.01;
    constexpr int min_line_length_coef = 30;
    constexpr int max_line_gap_coef = 15;

    constexpr float threshold_q = 10;
    constexpr int extended_length = 90;

    // 処理負荷の軽減のため、画像を小さくする
    constexpr int after_height = 300;
    const double f = double(after_height) / image.height();
    Image image_scaled = image.scaled(f);

    // 輪郭線以外の情報をなるべく消す。
    for (int i = 0; i < N; i++) {
        image_scaled.bilateralFilter(d, sigma_color, sigma_space);
    }
    // # 特徴線を抽出する。
    // 特徴線集合
    std::vector<cv::Vec4i> lines_from_pictures;
    {
        Image img_gray = image_scaled.grayscaled();

        // そのためのCanny, Hough変換を利用するべくいったんOpenCVのMatに変換する。
        cv::Mat img_grayed_mat = cv::Mat::zeros(img_gray.height(), img_gray.width(), CV_8U);
        for (int r = 0; r < img_gray.height(); r++){
            for (int c = 0; c < img_gray.width(); c++) {
                img_grayed_mat.at<uint8_t>(r, c) = img_gray[r][c].r;
            }
        }
        
        cv::Mat cannyed;
        cv::Canny(img_grayed_mat, cannyed, canny_min, canny_max);
        const int features_point_count = cv::countNonZero(cannyed);

        const auto threshold = int32_t(features_point_count * elected_coef);
        cv::HoughLinesP(
            cannyed, lines_from_pictures,
            1, M_PI/180,
            threshold,
            img_gray.height() / min_line_length_coef,
            img_gray.height() / max_line_gap_coef
        );
    }

    
    // line_used[i] == true: まだ線iが統合されていない
    // OpenCVで得られたデータをLine型に変換する。
    const uint64_t line_count = lines_from_pictures.size();
    Console << line_count;
    Array<bool> line_used(line_count, true);
    Array<Line> lines; lines.reserve(line_count);
    for (const cv::Vec4i& line:lines_from_pictures){
        lines.push_back({
            {line[0], line[1]}, {line[2], line[3]}
        });
    }

    // 各辺の組み合わせのうち、最も近くて角度が類似している辺を結合する。
    for (size_t i = 0; i < line_count; i++) {
        for (size_t j = i + 1; j < line_count; j++) {
            if (not line_used[i] or not line_used[j]) { continue; }
            
            const Line& L1 = lines[i];
            const Line& L2 = lines[j];
            // 傾きが十分に似ている曲線を得る。
            if (abs(L1.vector().getAngle() - L2.vector().getAngle()) >= M_PI / 12) { continue; }

            // L2の端点をL1の直線上に移動させる。
            // この地点で移動距離が十分に小さいもののみを統合対象とする。
            const Vec2 L1_direction = L1.vector().normalized();
            // L2をL1へ統合した後の位置。ただし、L1とL2の距離が十分に遠ければ、それらは統合しない。
            const Vec2 L2b_prime = (L2.begin - L1.begin).dot(L1_direction) * L1_direction + L1.begin;
            const Vec2 L2e_prime = (L2.end - L1.begin).dot(L1_direction) * L1_direction + L1.begin;
            if (
                (L2b_prime - L2.begin).lengthSq() > threshold_q*threshold_q
                or (L2e_prime - L2.end).lengthSq() > threshold_q*threshold_q
            ) {
                continue;
            }
            // L1軸への正射影
            const auto projection = [&](const Vec2& v) -> double { return (v - L1.begin).dot(L1_direction); };
            if (
                segment_distance(
                    projection(L1.begin), projection(L1.end),
                    projection(L2b_prime), projection(L2e_prime)
                ) > threshold_q
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
    Rect bottom{{0, 0}, {image_scaled.width(), image_scaled.height()}};
    // 底に線を追加
    tied_lines.emplace_back(bottom.bottom());
    Console << U"tied_lines: " << tied_lines.size();
    // もともと画像サイズを圧縮していたので、得られた線分の座標を画像座標系のそれに戻す。
    for (Line& line : tied_lines) {
        line.begin /= f;
        line.end /= f;
    }
    return tied_lines;
}
