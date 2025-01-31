#pragma once
#include <Siv3D.hpp>

#if true

namespace LandscapeWander::detector {

/**
     * @brief Fast Line Detectorによって足場を検出する。
     * 
     * @param image 
     * @param alpha 検出対象のコントラストの閾値
     * @param beta 検出する線分の微細さ
     * @param gamma 線分の結合度
     * @return Array<Line> 
     */
Array<Line> fld(
    const Image& image,
    const double alpha = 0.5,
    const double beta = 0.5,
    const double gamma = 0.5
);

}
#endif