#pragma once
# include <Siv3D.hpp>
# include "MainGame.hpp"
# include "../Utility/Animation.hpp"
# include "../Utility/numeric.hpp"
# include "../Utility/AnimationManager.hpp"
# include "../UIComponent/Layout.hpp"
# include "../LandscapeWander/image_process.hpp"

namespace LandscapeExtenders {

Vec2 MainGame::scroll_offset() const {
    return ClampXY(
        player.transform_.position + Vec2{player.transform_.velocity.x*0.5, 0},
        world_bounding_box.tl() + Camera_world_Rect() / 2,
        world_bounding_box.br() - Camera_world_Rect() / 2
    );
}

void MainGame::set_stage() {
    world.initialize();
    world_bounding_box = world.bounding_box();
}

MainGame::MainGame(const InitData& init):
    IScene{init},
    bgm_game{AudioAsset(U"bgm/walk-demo")},
    file_path(U"./assets/test/page3/ex2.png")
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
        collision_tickets.clear();
        if (world.hit(player.collision_line(), collision_tickets)) {
            for (const CollisionTicket& collision_ticket:collision_tickets) {
                player.resolve_collision(collision_ticket);
            }
        }
    }
    if (KeyA.down()) { displaying_line = not displaying_line; }
    // if (KeyB.down()) { changeScene(U"Title", 500); }
    if (KeyC.down()) {
        configure_mode = not configure_mode;
        if (not configure_mode and some_param_modified) {
            set_stage();
            some_param_modified = false;
        }
    }
    if (KeyR.down()) {
        player.transform_.position = this->player_inital_place();
        player.transform_.velocity = Vec2::Zero();
    }
    if (configure_mode or abs(player.transform_.velocity.x) > 0.1) { stop_time_stopwatch.restart(); }

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
        
        
        world.draw(visible_region());
        if (displaying_line){ world.draw_lines(visible_region()); }

        effect.update();
        player.draw();
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
    if (stop_time_stopwatch.s() >= 4) {
        const double opacity = std::min(double(stop_time_stopwatch.ms() - 4000)/1000, 1.0);
        const ScopedColorMul2D scm{1, 1, 1, opacity}; 
        const Rect info_rect = cliped_Y(Scene::Rect(), 0.75, 0.95);
        info_rect.draw(ColorF{0, 0, 0, 0.5});
        FontAsset(U"UIFont")(U"[A] 検出した線の表示切替, [R] やりなおし \n[←↑↓→]: 移動, [スペース]: ジャンプ")
        .draw(20, Arg::center = info_rect.center(), Palette::White);
    }

    const double t = 1 - EaseInOutExpo(board_transition.value());
    if (configure_mode or t > 0) {
        // アニメーション中の`configure_rect`の位置
        const Rect current_conrect = configure_rect.movedBy(configure_rect.w * 1.5 * t, 0);
        RoundRect{current_conrect, 5}.drawShadow({5, 5}, 3.0).draw(Palette::White);
        FontAsset(U"UIFont")(U"パラメータ設定").draw(25, current_conrect.stretched(-5).tl(), Palette::Black);
        
        RectSlicer rs { current_conrect.stretched(-5), RectSlicer::Axis::Y_axis };
        Rect rect;
        rs.from(0.2);
        
        const auto param_slider_setting = [&](
            const double position,
            const String& description,
            double& variable
        ) -> void {
            rect = rs.to(position);
            FontAsset(U"UIFont")(description).draw(20, rect.leftCenter(), Palette::Black);
            rect = rs.to(position + 0.1);
            if (SimpleGUI::Slider(U"{:.2f}"_fmt(variable), variable, rect.tl(), rect.w/5, rect.w*4/5, configure_mode)) {
                some_param_modified = true;
            }
        };

        param_slider_setting(0.3, U"α: 検出対象のコントラストの閾値", this->alpha);
        param_slider_setting(0.5, U"β: 検出する線分の微細さ", this->beta);
        param_slider_setting(0.7, U"γ: 線分の結合度", this->gamma);

        rect = rs.to(0.9);
        if (SimpleGUI::Button(U"写真をロードする", rect.tl(), rect.w, configure_mode)) {
            if (const auto path = Dialog::OpenFile({ FileFilter::AllImageFiles() }, U"../assets/test", U"ステージファイルをえらぼう")) {
                file_path = *path;
                some_param_modified = true;
            }
        }

        if (not current_conrect.mouseOver() and MouseL.up()) {
            if (some_param_modified) { set_stage(); }
            configure_mode = false;
        }
    }
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
