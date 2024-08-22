#include "EdgeDetectedStage.hpp"
#include "../LandscapeWander/image_process.hpp"

using std::make_shared;

namespace LandscapeExtenders{
EdgeDetectedStage::EdgeDetectedStage(
    const Firebase::StageData& stage_data,
    double alpha, double beta, double gamma
):
    info(stage_data)
{
    FilePath file_path = FileSystem::PathAppend(Firebase::stage_image_folderpath, stage_data.storageURI);
    image = Image{file_path};
    landscape = Texture{image};

    Array<Line> stage_line_on_photo = extract_stageline_from(image, alpha, beta, gamma);
    const double photo_meter_per_pixel = stage_data.area.w / image.width();
    for (Line& line : stage_line_on_photo) {
        Vec2 begin = line.begin * photo_meter_per_pixel + stage_data.area.tl();
        Vec2 end = line.end * photo_meter_per_pixel + stage_data.area.tl();
        terrain.push_back(make_shared<Line>(begin, end));
    }
}

}