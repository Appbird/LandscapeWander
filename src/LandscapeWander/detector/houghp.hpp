#pragma once
#include <Siv3D.hpp>

namespace LandscapeWander::detector {
    /**
     * @brief 確率的ハフ変換によって足場を検出する。
     * 
     * @param image 
     * @param alpha 検出対象のコントラストの閾値
     * @param beta 検出する線分の微細さ
     * @param gamma 線分の結合度
     * @return Array<Line> 
     */
    Array<Line> houghp(
        const Image& image,
        const double alpha = 0.5,
        const double beta = 0.5,
        const double gamma = 0.5
    );
}