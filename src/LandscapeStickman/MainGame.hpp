#pragma once
#include <Siv3D.hpp>
#include "../App.hpp"

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

        Array<Array<CollisionEvent>> collision_events(backgrounds.size());

    Effect effect;

        Font UI_font{36, Typeface::Heavy};

        const int page_number = 2;
        const int page_count = 5;

        GameState gamestate = G_Ready;
        Array<Vec2> backgrounds_offset;
        Array<Array<Line>> lines_of_stages;
        Array<Texture> background_textures;
    public:
        MainGame(const InitData& init);
        void update() override;
        void draw() const override;
};

}