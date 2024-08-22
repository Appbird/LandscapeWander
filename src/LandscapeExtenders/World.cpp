#pragma once
#include <Siv3D.hpp>
#include "World.hpp"
#include "../Firebase/FirebaseAPI.hpp"

namespace LandscapeExtenders {

static Firebase::StageList setup_stage_list();

void World::initialize() {
    Firebase::StageList stages_info = setup_stage_list();
    for (const auto& [key, stage_info]:stages_info) {
        auto stage_terrain = EdgeDetectedStage{stage_info, 0.5, 0.5, 0.5};
        stages.insert({key, stage_terrain});
    }   
}
void PhotoStage::draw() const {
    edge_detected_stage.info.area
        .drawFrame(0.5, Color{75, 119, 150})
        .drawShadow(Vec2{ 0, 0 }, 1, 0.5);
    edge_detected_stage.landscape
        .resized(edge_detected_stage.info.area.size)
        .draw(edge_detected_stage.info.area.tl());
}

static Firebase::StageList setup_stage_list() {
    Firebase::API api;
    api.initialize();
    Firebase::StageList stage_list = api.fetch_stage_array();
    Firebase::cache_stage_list(stage_list);
    for (const auto& [key, stage]:stage_list) {
        api.fetch_and_save_image(stage);
    }
    return stage_list;
}

}