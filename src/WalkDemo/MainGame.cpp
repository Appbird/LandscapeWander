#pragma once
# include <Siv3D.hpp>
# include "MainGame.hpp"
# include "../Utility/Animation.hpp"
# include "../Utility/numeric.hpp"
# include "../Utility/AnimationManager.hpp"
# include "../UIComponent/Layout.hpp"
# include "../LandscapeWander/image_process.hpp"

namespace WalkDemo {

Vec2 MainGame::scroll_offset() const {
    return ClampXY(
        player.transform_.position + Vec2{player.transform_.velocity.x*0.5, 0},
        Vec2::Zero() + Camera_world_Rect() / 2,
        Photo_world_Rect() - Camera_world_Rect() / 2
    );
}

void MainGame::set_stage() {
    background = Image{file_path};

    lines_of_stage = extract_stageline_from(background);
    background_texture = Texture{background};
    for (Line& line : lines_of_stage) {
        line.begin  *= photo_meter_per_pixel();
        line.end    *= photo_meter_per_pixel();
    }
    player.transform_.position = player_inital_place();
}

MainGame::MainGame(const InitData& init):
    IScene{init},
    bgm_game{AudioAsset(U"bgm/walk-demo")},
    file_path(U"../assets/test/page3/ex1.png")
{
    assert(bgm_game);
    // 背景の色を設定する
    Scene::SetBackground(ColorF{ 0.1, 0.1, 0.1 });
    set_stage();
    player.Init(player_inital_place());

    configure_rect = cliped_X(Scene::Rect(), 0.5, 0.97).stretched(-5);
    bgm_game.play();
    game_start_stopwatch.start();
}

void MainGame::update() {
    already_drawn = false;
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
    if (KeyB.down()) { changeScene(U"Title", 500); }
    if (KeyC.down()) {
        configure_mode = not configure_mode;
        if (not configure_mode and some_param_modified) {
            set_stage();
            some_param_modified = false;
        }
    }

    board_transition.update(configure_mode);
    draw_world();
    draw_UI();
    already_drawn = true;
 }
void MainGame::draw_world() const {
    {
        const ScopedRenderTarget2D screen{whole_blur_textures.blur1.clear(ColorF{1})};
        const Transformer2D centerized{Mat3x2::Translate(Scene::Center())};
        const Transformer2D scaled{Mat3x2::Scale(screen_pixel_per_meter())};
        const Transformer2D transformer{ Mat3x2::Translate(-scroll_offset()), TransformCursor::Yes };
        background_texture.resized(Photo_world_Rect()).draw({0, 0});
        
        effect.update();
        player.draw();
        if (displaying_line){
            for (const Line& line : lines_of_stage) {
                // #TODO ラインの描画方法について考える。
                line.draw(0.2, HSV{120, 0.4, 1, 0.7+ 0.2 * Periodic::Sine0_1(2s)});
            }
        }
        {
            const ScopedRenderTarget2D bloom_target{bloom_textures.blur1.clear(ColorF{0})};
            const ScopedColorMul2D colorMul{ ColorF{1, 0.2*Periodic::Jump0_1(1.0s) + 0.8} };
            player.draw();
        }
    }

    if (configure_mode) { Bloom(whole_blur_textures, false); } else { whole_blur_textures.blur1.draw(); }
    Bloom(bloom_textures);
    
}
void MainGame::draw_UI() {
    // UI関連
    const double t = 1 - EaseInOutExpo(board_transition.value());
    if (configure_mode or t > 0) {
        const Rect current_conrect = configure_rect.movedBy(configure_rect.w * 1.5 * t, 0);
        RoundRect{current_conrect, 5}.drawShadow({5, 5}, 3.0).draw(Palette::White);
        FontAsset(U"UIFont")(U"パラメータ設定").draw(25, current_conrect.stretched(-5).tl(), Palette::Black);
        
        RectSlicer rs { current_conrect.stretched(-5), RectSlicer::Axis::Y_axis };
        
        Rect rect1 = rs.from(0.2).to(0.3);
        if (SimpleGUI::Slider(U"α {:.2f}"_fmt(this->alpha), this->alpha, rect1.tl(), rect1.w/4, rect1.w*3/4, configure_mode)) {
            some_param_modified = true;
        }

        Rect rect2 = rs.to(0.4);
        if (SimpleGUI::Button(U"写真をロードする", rect2.tl(), rect2.w, configure_mode)) {
            if (const auto path = Dialog::OpenFile({ FileFilter::AllImageFiles() }, U"../assets/test", U"ステージファイルをえらぼう")) {
                file_path = *path;
                some_param_modified = true;
            }
        }

        Rect rect3 = rs.to(0.5);
        if (SimpleGUI::Button(U"検出結果を保存する", rect2.tl(), rect2.w, configure_mode)) {
            save_world();
        }

        if (not current_conrect.mouseOver() and MouseL.up()) { configure_mode = false; }
    }
}

void MainGame::save_world() const {
    Array<Line> lines_of_stage = extract_stageline_from(background);
    Image background_texture = background.cloned();
    {
        for (const Line& line : lines_of_stage) {
            line.overwrite(background_texture, 2, HSV{120, 0.4, 1, 1});
        }
    }
    background_texture.savePNG(U"../assets/test/result/" + FileSystem::BaseName(file_path) + U".png");
}

void MainGame::draw() const {
    if (not already_drawn) { draw_world(); }
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
