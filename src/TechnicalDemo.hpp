# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.11
# include "Animation.hpp"
# include "AnimationManager.hpp" 
# include "Player.hpp"
# include "CollisionEvent.hpp"
# include "image_process.hpp"
# include "Bloom.hpp"
# include "utility.hpp"
# include "RegisterResource.hpp"
void DemoMain()
{
    // 背景の色を設定する
	Scene::SetBackground(ColorF{ 0.6, 0.8, 0.7 });
    RegisterResource();

    Audio bgm{U"../assets/music/Clarity_of_My_Sight.mp3", Loop::Yes};
    assert(bgm);
    //bgm.playOneShot();

    BloomTextures bloom;
    Image background{ U"../assets/test/ex3.jpeg" };
    Array<Line> stage_lines = extract_stageline_from(background);
    Texture background_texture{ background };

    Vec2 position{4.0, 2.0};
    Player player{position};

    Effect effect;
    Array<CollisionEvent> collision_events;
    const double photo_world_width = 50;
    const double camera_world_width = 30;
    // カメラが写す画面の範囲(mを単位とする。)
    Vec2 Camera_world_Rect = {camera_world_width, camera_world_width * Scene::Height() / Scene::Width()};
    // 写真がワールド空間ではどれほどの大きさを持つか(mを単位とする。)
    Vec2 Photo_world_Rect = {photo_world_width, photo_world_width * background.size().y / background.size().x};
    const double pixel_per_meter = Scene::Width() / camera_world_width;
    const double photo_meter_per_pixel = (photo_world_width / background.width());
    for (Line& line : stage_lines) {
        line.begin  *= photo_meter_per_pixel;
        line.end    *= photo_meter_per_pixel;
    }

    bool display_line = true;
    
    while (System::Update()) {
        ClearPrint();
        player.update(effect);
        // 衝突情報の更新
        {
            collision_events.clear();
            for (const auto& [index, line] : IndexedRef(stage_lines)) {
                if (const auto collided_points = line.intersectsAt(player.collision_line())) {
                    collision_events.push_back({int(index), *collided_points});
                }
            }
        }
        player.resolve_collision(collision_events, stage_lines);   
        if (KeyA.down()) { display_line = not display_line; }
        const Vec2 scroll_offset = Clamp(
            player.transform_.position + Vec2{player.transform_.velocity.x*0.5, 0},
            Vec2::Zero() + Camera_world_Rect/2,
            Photo_world_Rect - Camera_world_Rect / 2
        );
        {
            const Transformer2D centerized{Mat3x2::Translate(Scene::Center())};
            const Transformer2D scaled{Mat3x2::Scale(pixel_per_meter)};
            const Transformer2D transformer{ Mat3x2::Translate(-scroll_offset), TransformCursor::Yes };
            background_texture.resized(Photo_world_Rect).draw(0, 0);
            
            effect.update();
            player.draw();
            if (display_line) {
                for (const Line& line : stage_lines) {
                    // #TODO ラインの描画方法について考える。
                    line.draw(LineStyle::RoundCap, 0.25, HSV{120, 0.4, 1, 0.4 + 0.1 * Periodic::Sine0_1(2s)});
                }
            }

            {
                const ScopedRenderTarget2D bloom_target{bloom.blur1.clear(ColorF{0})};
                const ScopedColorMul2D colorMul{ ColorF{1, 0.4*Periodic::Jump0_1(1.0s) + 0.6} };
                player.draw();
            }
        }
        Bloom(bloom);
    }
    
}
