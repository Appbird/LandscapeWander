#pragma once
#include <Siv3D.hpp>
#include "FirebaseAPI.hpp"
#include "../App.hpp"

namespace LandscapeExtenders {

// #TODO 操作説明も画面下部に加える。
// 特に、Aキー, Bキー

class MainGame : public App::Scene {
public:
    Firebase::API api;
    Firebase::StageList stage_list;
    
    MainGame(const InitData& init):
        IScene{init}
    {
        try {
            api.initialize();
            stage_list = api.fetch_stage_array();
            Firebase::cache_stage_list(stage_list);
            for (const auto& [key, stage]:stage_list) {
                api.fetch_and_save_image(stage);
            }
        } catch (Error e) {
            System::MessageBoxOK(U"エラーが発生しました。", e.what());
            exit(0);
        }
    }
    void update() override {
        ClearPrint();
        for (const auto& [key, stage] : stage_list) {
            Print << std::make_pair(key, stage.encode());
        }
    }
    void draw() const override {   
    }
};

}
