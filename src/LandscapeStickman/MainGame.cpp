#pragma once
# include <Siv3D.hpp>
# include "MainGame.hpp"
# include "../Utility/Animation.hpp"
# include "../Utility/numeric.hpp"
# include "../Utility/AnimationManager.hpp"
# include "../LandscapeWander/image_process.hpp"


namespace LandscapeStickman {

Vec2 MainGame::scroll_offset() const {
    return ClampY(
        blackhole.position - Vec2{Camera_world_Rect().x/2 - blackhole.basic_size.x / 2, 0},
        Vec2::Zero() + Camera_world_Rect() / 2,
        Photo_world_Rect() - Camera_world_Rect() / 2
    );
}

MainGame::MainGame(const InitData& init):
    IScene{init},
    bgm_game{AudioAsset(U"bgm/game")},
    se_bighit{AudioAsset(U"se/bighit")}
{
    assert(bgm_game);
    assert(se_bighit);

    // 背景の色を設定する
    Scene::SetBackground(ColorF{ 0.1, 0.1, 0.1 });

    const String extention{ U"png" };
    const int page_number = 2;
    const int page_count = 5;
    for (int i = 1; i <= page_count; i++) {
        backgrounds.push_back(Image{U"../assets/test/page" + Format(page_number) + U"/ex" + Format(i) + U"." + extention});
    }
    backgrounds_offset.resize(backgrounds.size());
    collision_events.resize(backgrounds.size());
    for (const Image& background : backgrounds){
        lines_of_stages.push_back(extract_stageline_from(background));
        background_textures.emplace_back(background);
    }
    for (Array<Line>& lines_of_stage: lines_of_stages){
        for (Line& line : lines_of_stage) {
            line.begin  *= photo_meter_per_pixel();
            line.end    *= photo_meter_per_pixel();
        }
    }
    for (size_t i = 1; i < lines_of_stages.size(); i++) {
        for (Line& line : lines_of_stages[i]) {
            line.begin.x  += Photo_world_Rect().x * i;
            line.end.x    += Photo_world_Rect().x * i;
        }
        backgrounds_offset[i].x += Photo_world_Rect().x * i;
    }

    player.Init({3.0, Photo_world_Rect().y/3});
    player.transform_.velocity.x = 20;
    blackhole.Init(
        {Camera_world_Rect().x - Camera_world_Rect().x/7, Photo_world_Rect().y/7},
        Vec2{Camera_world_Rect().x/10, Photo_world_Rect().y/5} * 2,
        Camera_world_Rect()
    );

    bgm_game.play();
    game_start_stopwatch.start();
}

void MainGame::update() {
    ClearPrint();
    player.update(effect);
    // 背景ループなど
    {
        // cameraの存在する場所
        const int photo_index_world = (int)floor( (blackhole.position.x - (Camera_world_Rect().x/2 + blackhole.basic_size.x/2)) / photo_world_width() ) - 2;
        // const int cycle_count = photo_index_world / backgrounds.size();
        const int photo_index_camera = ((photo_index_world) % backgrounds.size()) + ((photo_index_world % backgrounds.size() >= 0) ? 0 : backgrounds.size());
        
        if (photo_passing_count < photo_index_world) {
            if (photo_index_world >= 0) {
                for (Line& line : lines_of_stages[photo_index_camera]) {
                    line.begin.x  += backgrounds.size() * Photo_world_Rect().x;
                    line.end.x    += backgrounds.size() * Photo_world_Rect().x;
                }
                backgrounds_offset[photo_index_camera].x += backgrounds.size() * Photo_world_Rect().x;
            }
            photo_passing_count = photo_index_world;
        }
    }
    // 衝突情報の更新
    {
        assert(collision_events.size() == lines_of_stages.size());
        for (size_t i = 0; i < lines_of_stages.size(); i++) {
            collision_events[i].clear();
            // #FIXME
            for (const auto& [index, line] : IndexedRef(lines_of_stages[i])) {
                if (const auto collided_points = line.intersectsAt(player.collision_line())) {
                    collision_events[i].push_back({int(index), *collided_points});
                }
            }
        }
        if (const auto option_point = blackhole.collision_box().intersectsAt(player.collision_box())) {
            blackhole.shrink(player.transform_.velocity.lengthSq(), (*option_point)[0]);
            se_bighit.play();
            player.transform_.velocity *= -0.6;
            player.transform_.velocity.x = Clamp(player.transform_.velocity.x, -5.0, 1.0);
        }
    }
    for (size_t i = 0; i < lines_of_stages.size(); i++) {
        player.resolve_collision(collision_events[i], lines_of_stages[i]);
    }
    blackhole.update();

    
    const double left_edge_x = scroll_offset().x - Camera_world_Rect().x/2;
    // クリア処理
    if (blackhole.destroyed and player.should_running) {
        player.stop_running();
        player.controllable_state = false;
        gamestate = Success;
    }
    // ミス処理
    if (
        (blackhole.is_covering_all_region(left_edge_x) and player.should_running) or 
        (player.collision_box().topY() > Camera_world_Rect().y + 2 and player.should_running)
    ) {
        player.stop_running();
        player.controllable_state = false;
        gamestate = Failed;
    }
    
    if (is_game_end() and not game_end_stopwatch.isRunning()) { std::cout << "A" << std::endl; game_end_stopwatch.start(); }
    if (game_end_stopwatch.s() >= 6) {
        changeScene(U"LandscapeStickman/InstructionScene", 3000);
    }
}

void MainGame::draw() const 
{
    {
        const Transformer2D centerized{Mat3x2::Translate(Scene::Center())};
        const Transformer2D scaled{Mat3x2::Scale(screen_pixel_per_meter())};
        const Transformer2D transformer{ Mat3x2::Translate(-scroll_offset()), TransformCursor::Yes };
        for (size_t i = 0; i < background_textures.size(); i++) {
            background_textures[i].resized(Photo_world_Rect()).draw(backgrounds_offset[i]);
        }
        
        effect.update();
        player.draw();
        for (const Array<Line>& lines_of_stage:lines_of_stages) {
            for (const Line& line : lines_of_stage) {
                // #TODO ラインの描画方法について考える。
                line.draw(0.1, HSV{120, 0.4, 1, 0.5+ 0.2 * Periodic::Sine0_1(2s)});
            }
        }
        blackhole.draw();
        {
            const ScopedRenderTarget2D bloom_target{bloom_textures.blur1.clear(ColorF{0})};
            const ScopedColorMul2D colorMul{ ColorF{1, 0.2*Periodic::Jump0_1(1.0s) + 0.8} };
            player.draw();
        }
    }
    

    Bloom(bloom_textures);
    // UI関連
    {
        if (gamestate != Playing) {
            // リザルト表示
            RectF{{0, Scene::Height() * 0.4}, {Scene::Width(), Scene::Height() * 0.2}}.draw(ColorF{0, 0.5});
            FontAsset(U"UIFont")(
                (gamestate == Success) ? U"町を救った!" : U"Failed..."
            ).drawAt(Scene::Center(), Palette::White);
        } else {
            RectF{{0, 0}, {Scene::Width() * blackhole.destroyed_rate(), Scene::Height() / 20}}.draw(
                    HSV{20, 0.7, 0.70, 0.9}
            );
        }
    }
}

}

// Completed #TODO リソースを全てリソースマネジャに管理を統一させる
    // ref. https://zenn.dev/reputeless/books/siv3d-documentation/viewer/tutorial-asset
// #TODO SceneManagerを用いて実装する
    // update, drawを実装する形にする
    // ref. https://zenn.dev/reputeless/books/siv3d-documentation/viewer/tutorial-scene-manager
// #TODO タイトルを実装
    // ref. https://unityroom.com/games/bgmemory
// #TODO シーン遷移を丁寧にする
