#pragma once
#include <Siv3D.hpp>
#include "../Firebase/FirebaseAPI.hpp"

namespace LandscapeExtenders{
struct EdgeDetectedStage {
    Firebase::StageData info;
    Image image;
    Texture landscape;
    Array<std::shared_ptr<Line>> terrain;
    EdgeDetectedStage(
        const Firebase::StageData& stage_data,
        double alpha, double beta, double gamma
    );
    
};
}