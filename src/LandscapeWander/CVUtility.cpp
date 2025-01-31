#include "CVUtility.hpp"
#include "../Utility/numeric.hpp"

cv::Mat image_to_gray_mat(const Image& image_scaled) {
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

Array<Line> vectorCvVec4_to_arrayLine(const std::vector<cv::Vec4i>& lines_from_pictures) {
    const uint64_t line_count = lines_from_pictures.size();
    Array<Line> lines; lines.reserve(line_count);
    for (const cv::Vec4i& line:lines_from_pictures){
        lines.push_back({
            {line[0], line[1]}, {line[2], line[3]}
        });
    }
    return lines;
}

void save_line_detection(const Image& image, const Array<Line>& lines) {
    Image background_texture = image.cloned();
    {
        for (const Line& line : lines) {
            line.overwrite(background_texture, 2, HSV{120, 0.4, 1, 1});
        }
    }
    background_texture.savePNG(U"../result/final.png");
}

Array<Line> combine_lines(
    const Array<Line>& given_lines,
    const int32_t image_height,
    const double gamma,
    const double base_param
) {
    assert(0 < gamma and gamma < 1);

    const double angle_threshold = lerp(gamma, M_PI / 4, base_param);
    const double transverse_threshold = image_height * lerp(gamma, 0.04, base_param);
    const double longitudinal_threshold = image_height *  lerp(gamma, 0.02, base_param);

    Array<Line> lines = given_lines;
    // line_used[i] == true: まだ線iが統合されていない
    // OpenCVで得られたデータをLine型に変換する。
    const size_t line_count = lines.size();
    Array<bool> line_used(line_count, true);
    // 各辺の組み合わせのうち、最も近くて角度が類似している辺を結合する。
    for (size_t i = 0; i < line_count; i++) {
        for (size_t j = i + 1; j < line_count; j++) {
            if (not line_used[i] or not line_used[j]) { continue; }
            Line& L1 = lines[i];
            Line& L2 = lines[j];
            if (L1.lengthSq() < L2.lengthSq()) { std::swap(L1, L2); }
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

