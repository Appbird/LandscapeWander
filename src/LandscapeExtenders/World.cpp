#pragma once
#include <Siv3D.hpp>
#include "World.hpp"
#include "../Firebase/FirebaseAPI.hpp"

namespace LandscapeExtenders {

static Firebase::StageList setup_stage_list(bool load_from_remote);

void World::initialize(
    const double alpha,
    const double beta,
    const double gamma, 
    bool load_from_remote
) {
    stages_info = setup_stage_list(load_from_remote);
    stages.clear();
    visited.clear();
    for (const auto& [key, stage_info]:stages_info) {
        auto stage_terrain = EdgeDetectedStage{stage_info, alpha, beta, gamma};
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

void World::reset_stage(
    const double alpha,
    const double beta,
    const double gamma,
    const String& where
) {
    if (not stages.contains(where)) { return; }
    auto stage_info = stages_info.at(where);
    auto stage_terrain = EdgeDetectedStage{stage_info, alpha, beta, gamma};
    stages.at(where) = stage_terrain;
}

static Firebase::StageList setup_stage_list(bool load_from_remote) {
    Firebase::API api;
    Firebase::StageList stage_list;
    try {
        api.initialize();
        stage_list = api.fetch_stage_array();
        const Firebase::StageList previous_stage_list = Firebase::load_cached_stage_list();
        Firebase::cache_stage_list(stage_list);
        for (const auto& [key, stage]:stage_list) {
            if (previous_stage_list.contains(key)) { continue; }
            api.fetch_and_save_image(stage);
        }
    } catch (Error e) {
        Console << U"Firebaseとの接続に失敗しました。キャッシュからのデータでゲームを進めます";
        Console << e;
        stage_list = Firebase::load_cached_stage_list();
    }
    return stage_list;
}

}