#pragma once
#include <Siv3D.hpp>
#include "../App.hpp"
#include "../Firebase/FirebaseAPI.hpp"
#include "../Utility/Bloom.hpp"
#include "World.hpp"
#include "Player.hpp"
namespace LandscapeExtenders {

// #TODO 操作説明も画面下部に加える。
// 特に、Aキー, Bキー


class MainGame : public App::Scene {
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
private:
    enum GameState {
        Playing,
        End
    };
    Audio bgm_game;

    Effect effect;
    BloomTextures bloom_textures;
    BloomTextures whole_blur_textures;
    Rect configure_rect;

    GameState gamestate = Playing;
    Array<CollisionTicket> collision_tickets;
    
    World world;
    RectF world_bounding_box;
    Player player;
    double alpha = 0.5;
    double beta = 0.5;
    double gamma = 0.5;

    bool displaying_line;
    bool configure_mode = false;
    bool some_param_modified = false;
    bool already_drawn = false;
    
    double _camera_world_height = 30.0;

    String file_path;

    Stopwatch game_start_stopwatch{StartImmediately::No};
    Stopwatch stop_time_stopwatch{StartImmediately::Yes};
    Transition board_transition{0.5s, 0.5s};

    double aspect_ratio() const {
        return (double)Scene::Width() / Scene::Height();
    }
    // カメラが写す世界の大きさをm単位で返す。
    Vec2 Camera_world_Rect() const {
        return {camera_world_height() * aspect_ratio() , camera_world_height()};
    }
    // ゲーム空間中でのカメラ座標系の背景の高さをm単位で返す。
    double camera_world_height() const {
        return _camera_world_height;
    }
    
    // ゲームが終了したかを返す。
    bool is_game_end() const {
        return gamestate == End;
    }
    // 写真の横幅がゲーム空間の何メートルを占めるかを表す。(mを単位とする。)
    Vec2 player_inital_place() const {
        // #TODO 後でいい感じの場所に変えておく。
        return world.initial_start_point();
    }
    double screen_pixel_per_meter() const {
        return Scene::Height() / camera_world_height();
    }
    // 現在のスクロール量を返す。
    Vec2 scroll_offset() const;
    void set_stage();
    RectF visible_region() const {
        return {
            Arg::center = scroll_offset(),
            Camera_world_Rect()
        };
    }
};

}
