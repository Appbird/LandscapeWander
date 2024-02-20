#pragma once
# include <Siv3D.hpp>
# include "MainGame.hpp"
# include "../Utility/Animation.hpp"
# include "../Utility/numeric.hpp"
# include "../Utility/AnimationManager.hpp"
# include "../LandscapeWander/image_process.hpp"


namespace WalkDemo {

Vec2 MainGame::scroll_offset() const {
    return ClampXY(
        player.transform_.position + Vec2{player.transform_.velocity.x*0.5, 0},
        Vec2::Zero() + Camera_world_Rect() / 2,
        Photo_world_Rect() - Camera_world_Rect() / 2
    );
}

MainGame::MainGame(const InitData& init):
    IScene{init},
    bgm_game{AudioAsset(U"bgm/walk-demo")}
{
    assert(bgm_game);

    // 背景の色を設定する
    Scene::SetBackground(ColorF{ 0.1, 0.1, 0.1 });

    const String extention{ U"png" };
    background = Image{U"../assets/test/ex1.png"};

    lines_of_stage = extract_stageline_from(background);
    background_texture = Texture{background};
    for (Line& line : lines_of_stage) {
        line.begin  *= photo_meter_per_pixel();
        line.end    *= photo_meter_per_pixel();
    }

    player.Init({3.0, Photo_world_Rect().y/3});

    bgm_game.play();
    game_start_stopwatch.start();
}

void MainGame::update() {
    player.update(effect);
    // 衝突情報の更新
    {
        collision_events.clear();
        for (const auto& [index, line] : IndexedRef(lines_of_stage)) {
            if (const auto collided_points = line.intersectsAt(player.collision_line())) {
                collision_events.push_back({int(index), *collided_points});
            }
        }
    }
    player.resolve_collision(collision_events, lines_of_stage);   
    if (KeyA.down()) { displaying_line = not displaying_line; }
    if (KeyB.down()) {
        changeScene(U"LandscapeStickman/InstructionScene", 3000);
    }
}

void MainGame::draw() const 
{
    {
        const Transformer2D centerized{Mat3x2::Translate(Scene::Center())};
        const Transformer2D scaled{Mat3x2::Scale(screen_pixel_per_meter())};
        const Transformer2D transformer{ Mat3x2::Translate(-scroll_offset()), TransformCursor::Yes };
        background_texture.resized(Photo_world_Rect()).draw({0, 0});
        
        effect.update();
        player.draw();
        if (displaying_line){
            for (const Line& line : lines_of_stage) {
                // #TODO ラインの描画方法について考える。
                line.draw(0.1, HSV{120, 0.4, 1, 0.5+ 0.2 * Periodic::Sine0_1(2s)});
            }
        }
        {
            const ScopedRenderTarget2D bloom_target{bloom_textures.blur1.clear(ColorF{0})};
            const ScopedColorMul2D colorMul{ ColorF{1, 0.2*Periodic::Jump0_1(1.0s) + 0.8} };
            player.draw();
        }
    }

    Bloom(bloom_textures);
    // UI関連
    {
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
