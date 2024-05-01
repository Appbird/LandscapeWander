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
        GS_Ready,
        Playing,
        Success,
        Failed
    };
    Audio bgm_game;
    Audio se_bighit;
    Array<Image> backgrounds;
    Array<Texture> background_textures;
    Array<TextureRegion> background_texture_regions;
    
    Effect effect;
    BloomTextures bloom_textures;

    GameState gamestate = GS_Ready;
    Array<Array<Line>> lines_of_stages;
    Array<Array<CollisionEvent>> collision_events;
    
    Player player;
    Blackhole blackhole;
    
    Stopwatch game_start_stopwatch  { StartImmediately::No };
    Stopwatch game_end_stopwatch    { StartImmediately::No };

    // photo_meter_accm[i]: i番目の写真の、ゲーム空間上での大きさを示す。
    std::vector<Size> photo_world_position;
    // photo_meter_accm[i]: i番目の写真まで通過したときに右端がループスタート地点から何メートル離れているかを表す。
    std::vector<double> photo_lines_offsets;
    size_t photo_index_count = 0;
    // カメラが写す画面の範囲(mを単位とする。)
    double photo_world_height() const { 
        return 45;
    }
    // ゲーム空間中での写真背景の高さをm単位で返す。
    double camera_world_height() const { return photo_world_height(); }
    // カメラが写す世界の大きさをm単位で返す。
    Vec2 Camera_world_Rect() const {
        return {camera_world_height() * Scene::Width() / Scene::Height(), camera_world_height()};
    }
    // ゲーム空間中での写真背景の大きさをm単位で返す。
    Vec2 Photo_world_Rect(size_t index) const {
        assert(backgrounds.size() > 0);
        return {
            photo_world_height() * backgrounds[index].size().x / backgrounds[index].size().y,
            photo_world_height()
        };
    }
    
    // 写真の1ピクセルの幅がゲーム空間の何メートルを占めるかを返す。
    double photo_meter_per_pixel(size_t index) const {
        assert(0 <= index and index < backgrounds.size());
        return (photo_world_height() / backgrounds[index].height());
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
    // 写真集を一周するのに必要なメートル数
    double photo_period_meter() const {
        return photo_lines_offsets.back();
    }
    double photo_start_position(const size_t index) const {
        return photo_lines_offsets[index % photo_count()] + (index / photo_count()) * photo_period_meter();
    }
    // 写真集に含まれた写真の個数
    size_t photo_count() const {
        return background_textures.size();
    }
public:
    MainGame(const InitData& init);
    void update() override;
    void draw() const override;
    ~MainGame() {
        bgm_game.stop();
        se_bighit.stop();
    }
};

}
