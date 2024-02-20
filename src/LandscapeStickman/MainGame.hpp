#pragma once
#include <Siv3D.hpp>
#include "../App.hpp"
#include "../Utility/Bloom.hpp"
#include "../Utility/CollisionEvent.hpp"
#include "Player.hpp"
#include "Blackhole.hpp"
namespace LandscapeStickman {

class MainGame : public App::Scene {
private:
    enum GameState {
        Playing,
        Success,
        Failed
    };
    Audio bgm_game;
    Audio se_bighit;
    Array<Image> backgrounds;
    Array<Texture> background_textures;
    
    Effect effect;
    BloomTextures bloom_textures;

    GameState gamestate = Playing;
    Array<Vec2> backgrounds_offset;
    Array<Array<Line>> lines_of_stages;
    Array<Array<CollisionEvent>> collision_events;
    
    Player player;
    Blackhole blackhole;
    
    int photo_passing_count = -2;
    Stopwatch game_start_stopwatch{StartImmediately::No};
    Stopwatch game_end_stopwatch{StartImmediately::No};



    // カメラが写す画面の範囲(mを単位とする。)
    double photo_world_width() const {
        return 50;
    }
    // カメラが写す世界の大きさをm単位で返す。
    Vec2 Camera_world_Rect() const {
        return {camera_world_height() * Scene::Width() / Scene::Height(), camera_world_height()};
    }
    // ゲーム空間中での写真背景の大きさをm単位で返す。
    Vec2 Photo_world_Rect() const {
        assert(backgrounds.size() > 0);
        return {photo_world_width(), photo_world_width() * backgrounds[0].size().y / backgrounds[0].size().x};
    }
    // ゲーム空間中での写真背景の高さをm単位で返す。
    double camera_world_height() const {
        return Photo_world_Rect().y;
    }
    // 写真の1ピクセルの幅がゲーム空間の何メートルを占めるかを返す。
    double photo_meter_per_pixel() const {
        assert(backgrounds.size() > 0);
        return (photo_world_width() / backgrounds[0].width());
    }
    // ゲーム空間の1mがスクリーンの何ピクセル分の幅を占めているかを返す。
    double screen_pixel_per_meter() const {
        return Scene::Height() / camera_world_height();
    }
    bool is_game_end() const {
        return gamestate == Success or gamestate == Failed;
    }

    // 現在のスクロール量を返す。
    Vec2 scroll_offset() const;
public:
    MainGame(const InitData& init);
    void update() override;
    void draw() const override;
};

}
