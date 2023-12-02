# include <Siv3D.hpp> // OpenSiv3D v0.6.11
# include "Animation.hpp"
# include "AnimationManager.hpp" 
# include "Player.hpp"
# include "CollisionEvent.hpp"
# include "image_process.hpp"

Vec2 Clamp(const Vec2& value, const Vec2& min, const Vec2& max) {
    return Vec2{
        Clamp(value.x, min.x, max.x),
        Clamp(value.y, min.y, max.y),
    };
}
struct BloomTextures {
    const RenderTexture blur1;
    const RenderTexture internal1;
    const RenderTexture blur4;
    const RenderTexture internal4;
    const RenderTexture blur8;
    const RenderTexture internal8;
    BloomTextures():
        blur1{Scene::Size()},
        internal1{Scene::Size()},
        blur4{Scene::Size() / 4},
        internal4{Scene::Size() / 4},
        blur8{Scene::Size() / 8},
        internal8{Scene::Size() / 8}
    {}      
};

void Bloom(BloomTextures& bloom_texures) {
    {
        Shader::GaussianBlur(bloom_texures.blur1, bloom_texures.internal1, bloom_texures.blur1);
        
        Shader::Downsample(bloom_texures.blur1, bloom_texures.blur4);
        Shader::GaussianBlur(bloom_texures.blur4, bloom_texures.internal4, bloom_texures.blur4);

        Shader::Downsample(bloom_texures.blur4, bloom_texures.blur8);
        Shader::GaussianBlur(bloom_texures.blur8, bloom_texures.internal8, bloom_texures.blur8);
    }
    {
        const ScopedRenderStates2D blend{BlendState::Additive};
        bloom_texures.blur4.resized(Scene::Size()).draw(ColorF{0.4});
        bloom_texures.blur8.resized(Scene::Size()).draw(ColorF{0.5});
    }

}

void Main()
{
    // 背景の色を設定する
	Scene::SetBackground(ColorF{ 0.6, 0.8, 0.7 });
    
    Audio bgm{U"../assets/music/Clarity_of_My_Sight.mp3", Loop::Yes};
    assert(bgm);
    //bgm.playOneShot();

    BloomTextures bloom;
    Image background{ U"../assets/test/ex1.png" };
    Array<Line> stage_lines = extract_stageline_from(background);
    Texture background_texture{ background };

    Vec2 position{0.5, 0.5};
    Player player{position};

    Effect effect;
    Array<CollisionEvent> collision_events;
    const double photo_world_width = 40;
    const double camera_world_width = 20;
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
            for (const Line& line : stage_lines) {
                // #TODO ラインの描画方法について考える。
                line.draw(0.1, HSV{120, 0.4, 1, 0.05 + 0.2 * Periodic::Sine0_1(2s)});
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
