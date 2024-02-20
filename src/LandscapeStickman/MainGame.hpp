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
            G_Ready,
            Playing,
            Success,
            Failed
        };
        Audio bgm_game;
        Audio bgm_instruction;
        Audio se_bighit;
        
        Array<Texture> how_to_play;
        BloomTextures bloom;
        Array<Image> backgrounds;
        Effect effect;


        Font UI_font{36, Typeface::Heavy};

        const int page_number = 2;
        const int page_count = 5;
        const double photo_world_width = 50;
        const Vec2 initial_position{1, 10};

        GameState gamestate = G_Ready;
        Array<Vec2> backgrounds_offset;
        Array<Array<Line>> lines_of_stages;
        Array<Texture> background_textures;
        Array<Array<CollisionEvent>> collision_events;
        // カメラが写す画面の範囲(mを単位とする。)
        Vec2 Camera_world_Rect;
        Vec2 Photo_world_Rect;

        Player player;
        Blackhole blackhole;

        double camera_world_height;
        double photo_meter_per_pixel;
        double pixel_per_meter;
        int photo_passing_count = -2;
        size_t page = 0;

        Stopwatch ready_state{StartImmediately::No};
        Stopwatch game_start_stopwatch{StartImmediately::No};
        void Init();
    public:
        MainGame(const InitData& init): IScene{init} {
            Init();
        }
        void update() override;
        void draw() const override;
};

}
