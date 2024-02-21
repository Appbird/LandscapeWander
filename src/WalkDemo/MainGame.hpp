#pragma once
#include <Siv3D.hpp>
#include "../App.hpp"
#include "../Utility/Bloom.hpp"
#include "../Utility/CollisionEvent.hpp"
#include "Player.hpp"
namespace WalkDemo {

// #TODO 操作説明も画面下部に加える。
// 特に、Aキー, Bキー

class MainGame : public App::Scene {
private:
    enum GameState {
        Playing,
        End
    };
    Audio bgm_game;
    Image background;
    Texture background_texture;

    Effect effect;
    BloomTextures bloom_textures;
    BloomTextures whole_blur_textures;
    Rect configure_rect;

    GameState gamestate = Playing;
    Vec2 backgrounds_offset;
    Array<Line> lines_of_stage;
    Array<CollisionEvent> collision_events;
    
    Player player;
    double alpha = 0.5;

    bool displaying_line;
    bool configure_mode = false;
    bool some_param_modified = false;
    bool already_drawn = false;
    
    String file_path;

    Stopwatch game_start_stopwatch{StartImmediately::No};
    Transition board_transition{0.5s, 0.5s};

    // 写真の横幅がゲーム空間の何メートルを占めるかを表す。(mを単位とする。)
    Vec2 player_inital_place() const {
        return {Photo_world_Rect().x/2, Photo_world_Rect().y/4};
    }
    // 写真の横幅がゲーム空間の何メートルを占めるかを表す。(mを単位とする。)
    double photo_world_width() const {
        return 60;
    }
    // カメラが写す世界の大きさをm単位で返す。
    Vec2 Camera_world_Rect() const {
        return {camera_world_height() * Scene::Width() / Scene::Height(), camera_world_height()};
    }
    // ゲーム空間中での写真背景の大きさをm単位で返す。
    Vec2 Photo_world_Rect() const {
        return {photo_world_width(), photo_world_width() * background.size().y / background.size().x};
    }
    // ゲーム空間中でのカメラ座標系の背景の高さをm単位で返す。
    double camera_world_height() const {
        return Photo_world_Rect().y / 1.5;
    }
    // 写真の1ピクセルの幅がゲーム空間の何メートルを占めるかを返す。
    double photo_meter_per_pixel() const {
        return (photo_world_width() / background.width());
    }
    // ゲーム空間の1mがスクリーンの何ピクセル分の幅を占めているかを返す。
    double screen_pixel_per_meter() const {
        return Scene::Height() / camera_world_height();
    }
    bool is_game_end() const {
        return gamestate == End;
    }

    // 現在のスクロール量を返す。
    Vec2 scroll_offset() const;
    void set_stage();
public:
    MainGame(const InitData& init);
    void update() override;
    void draw() const override;
    void draw_world() const;
    void save_world() const;
    void draw_UI();
    ~MainGame() {
        bgm_game.stop();
    }
};

}
