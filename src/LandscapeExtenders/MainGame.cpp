#pragma once
# include <Siv3D.hpp>
# include "MainGame.hpp"
# include "../Utility/Animation.hpp"
# include "../Utility/numeric.hpp"
# include "../Utility/AnimationManager.hpp"
# include "../Utility/Notificator.hpp"
# include "../UIComponent/Layout.hpp"

namespace LandscapeExtenders {

Vec2 MainGame::scroll_offset() const {
    const Vec2 camera_position = player.transform_.position + Vec2{player.transform_.velocity.x*0.5, 0};
    const Vec2 v1 = world_bounding_box.tl() + Camera_world_Rect() / 2;
    const Vec2 v2 = world_bounding_box.br() - Camera_world_Rect() / 2;
    if (v1.y > v2.y or v1.x > v2.x) { return camera_position; }
    return ClampXY(camera_position, v1, v2);
}


void MainGame::set_stage() {
    world.initialize(alpha, beta, gamma, load_from_remote);
    world_bounding_box = world.bounding_box().stretched(10);
}

void MainGame::set_domestic_stage() {
    world.reset_stage(alpha, beta, gamma, world.where(player.foot_point()));
}


MainGame::MainGame(const InitData& init):
    IScene{init},
    bgm_game{AudioAsset(U"bgm/walk-demo")}
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

static bool pressed_line_display(Player::ControllMethod method) {
    switch (method) {
        case Player::ControllMethod::JoyCon:
            if (const auto joycon = JoyCon(0)) {
                return joycon.button3.down();
            }
        case Player::ControllMethod::Key:
            return KeyQ.down();
    }
    return false;
}
static bool pressed_controll_panel(Player::ControllMethod method) {
    switch (method) {
        case Player::ControllMethod::JoyCon:
            if (const auto joycon = JoyCon(0)) {
                return joycon.buttonZLZR.down();
            }
        case Player::ControllMethod::Key:
            return KeyC.down();
    }
    return false;
}
static bool pressed_shrink(Player::ControllMethod method) {
    switch (method) {
        case Player::ControllMethod::JoyCon:
            if (const auto joycon = JoyCon(0)) {
                return joycon.buttonSL.pressed();
            }
        case Player::ControllMethod::Key:
            return KeyS.pressed();
    }
    return false;
}
static bool pressed_scale_plus(Player::ControllMethod method) {
    switch (method) {
        case Player::ControllMethod::JoyCon:
            if (const auto joycon = JoyCon(0)) {
                return joycon.buttonSR.pressed();
            }
        case Player::ControllMethod::Key:
            return KeyD.pressed();
    }
    return false;
}
static bool down_retry(Player::ControllMethod method) {
    switch (method) {
        case Player::ControllMethod::JoyCon:
            if (const auto joycon = JoyCon(0)) {
                return joycon.buttonPlus.down();
            }
        case Player::ControllMethod::Key:
            return KeyR.down();
    }
    return false;
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
    if (pressed_line_display(player.CONTROLL_METHOD)) {
        displaying_line = not displaying_line;
        NotificationAddon::Show(
            displaying_line ?
                U"検出された足場を表示します。" : U"検出された足場を表示しません。"
            , NotificationAddon::Type::Information
        );
        
    }
    if (pressed_controll_panel(player.CONTROLL_METHOD)) {
        configure_mode = not configure_mode;
    }
    if (KeyY.down()) {
        changeScene(U"Title", 1000);
    }
    if (down_retry(player.CONTROLL_METHOD) or player.head_point().y > world_bounding_box.bottomY() + 10) {
        player.transform_.position = this->player_inital_place();
        player.transform_.velocity = Vec2::Zero();
        NotificationAddon::Show(U"プレイヤーが初期位置に戻されました。", NotificationAddon::Type::Information);
    }
    if (pressed_shrink(player.CONTROLL_METHOD)) { _camera_world_height -= 25 * Scene::DeltaTime(); }
    if (pressed_scale_plus(player.CONTROLL_METHOD)) { _camera_world_height += 25 * Scene::DeltaTime(); }
    _camera_world_height = Clamp(_camera_world_height, 10.0, 100.0);

    if (configure_mode or abs(player.transform_.velocity.x) > 0.1) { stop_time_stopwatch.restart(); }

    if (KeyColon_JIS.down()) {
        load_from_remote = not load_from_remote;
        NotificationAddon::Show(
            load_from_remote ?
            U"リモートの更新に追従するようにします"
            : U"リモート更新に追従しないようにします。",
            NotificationAddon::Type::Information
        );
    }

    world.visit(player.head_point());
    board_transition.update(configure_mode);
    draw_world();
    draw_UI();
    already_drawn = true;
    if (not configure_mode and some_param_modified) {
        if (load_from_remote) { set_stage(); } else { set_domestic_stage(); }
        some_param_modified = false;
        NotificationAddon::Show(
            U"足場が新たに検出され直しました。",
            NotificationAddon::Type::Success
        );
    }
 }
void MainGame::draw_world() const {
    {
        const ScopedRenderTarget2D screen{whole_blur_textures.blur1.clear(ColorF{1})};
        const Transformer2D centerized{Mat3x2::Translate(Scene::Center())};
        const Transformer2D scaled{Mat3x2::Scale(screen_pixel_per_meter())};
        const Transformer2D transformer{ Mat3x2::Translate(-scroll_offset()), TransformCursor::Yes };
        world_bounding_box.draw(Arg::topRight = Color{47, 63, 79}, Arg::bottomLeft = Color{38, 52, 66});
        
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
    if (stop_time_stopwatch.s() >= 2) {
        const double opacity = std::min(double(stop_time_stopwatch.ms() - 2000)/1000, 1.0);
        const ScopedColorMul2D scm{1, 1, 1, opacity}; 
        const Rect info_rect = cliped_Y(Scene::Rect(), 0.75, 0.95);
        info_rect.draw(ColorF{0, 0, 0, 0.5});
        const String description_key = U"[Q] 検出した線の表示切替 [S/D] 拡大/縮小 [R] やりなおし \n[←↑↓→]: 移動 [スペース]: ジャンプ";
        const String description_joycon = U"[Y] 検出した線の表示切替 [SR/SL] 拡大/縮小 [+/-] やりなおし \n[スティック]: 移動 [X]: ジャンプ";
        const String howto_discription = player.CONTROLL_METHOD == Player::ControllMethod::JoyCon ? description_joycon : description_key;
        FontAsset(U"UIFont")(howto_discription)
        .draw(20, Arg::center = info_rect.center(), Palette::White);
    }
    
    const double t = 1 - EaseInOutExpo(board_transition.value());
    if (configure_mode or t > 0) {
        // アニメーション中の`configure_rect`の位置
        const Rect current_conrect = configure_rect.movedBy(configure_rect.w * 1.5 * t, 0);
        RoundRect{current_conrect, 5}.drawShadow({5, 5}, 3.0).draw(Palette::White);
        FontAsset(U"UIFont")(U"パラメータ設定").draw(25, current_conrect.stretched(-5).tl(), Palette::Black);
        
        RectSlicer rs{ current_conrect.stretched(-5), RectSlicer::Axis::Y_axis };
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
        FontAsset(U"UIFont")(
            load_from_remote ? U"ステージ全体が更新されます" : U"今いる場所だけが更新されます"
        ).draw(20, rect.leftCenter(), Palette::Black);
        if (not current_conrect.mouseOver() and MouseL.up()) {
            configure_mode = false;
            
        }
    }

    const Rect visited_region = clipped(Scene::Rect(), RectF{0.9, 0.9, 0.1,});
    visited_region.draw(ColorF{0, 0, 0, 0.3});
    FontAsset(U"UIFont")(
        U"{} / {}"_fmt(world.visited.size(), world.count_stages())
    ).draw(20.0, Arg::center=visited_region.center(), Palette::White);
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
