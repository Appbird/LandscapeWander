# include <Siv3D.hpp>
# include "MainGame.hpp"
# include "../Utility/Animation.hpp"
# include "../Utility/numeric.hpp"
# include "../Utility/AnimationManager.hpp"
# include "../LandscapeWander/detector/fld.hpp"


namespace LandscapeStickman {

Vec2 MainGame::scroll_offset() const {
    return ClampY(
        blackhole.position - Vec2{Camera_world_Rect().x/2 - blackhole.basic_size.x / 2, 0},
        Vec2::Zero() + Camera_world_Rect() / 2,
        Photo_world_Rect(0) - Camera_world_Rect() / 2
    );
}

struct StageInformation {
    FilePath path;
    double alpha;
    double beta;
    double gamma;
};

MainGame::MainGame(const InitData& init):
    IScene{init},
    bgm_game{AudioAsset(U"bgm/game")},
    se_bighit{AudioAsset(U"se/bighit")}
{
    assert(bgm_game);
    assert(se_bighit);

    // 背景の色を設定する
    Scene::SetBackground(ColorF{ 0.1, 0.1, 0.1 });

    Array<StageInformation> stages_info = {
        {
            U"./assets/test/long1.jpg",
            0.40, 0.60, 0.40
        },
        {
            U"./assets/test/long2.png",
            0.40, 0.60, 0.40
        }
    };
    if (stages_info.size() == 0) { exit(0); }
    for (const auto& stage_info:stages_info) {
        backgrounds.push_back(Image{stage_info.path});
    }
    
    const size_t image_count = backgrounds.size();
    // 線分検知処理
    {
        collision_events.resize(image_count);
        for (const auto [index, background] : IndexedRef(backgrounds)){
            const auto& [path, alpha, beta, gamma] = stages_info[index];
            lines_of_stages.push_back(LandscapeWander::detector::fld(background, alpha, beta, gamma));
            background_textures.emplace_back(background);
        }
    }
    // スクロール位置に合わせた線分のスケーリングと位置シフト
    {
        for (const auto [index, lines_of_stage]: IndexedRef(lines_of_stages)){
            for (Line& line : lines_of_stage) {
                line.begin  *= photo_meter_per_pixel(index);
                line.end    *= photo_meter_per_pixel(index);
            }
        }
        photo_lines_offsets.resize(image_count + 1);
        photo_lines_offsets[0] = 0;
        for (int i = 0; i < int32_t(image_count); i++) {
            photo_lines_offsets[i + 1] = photo_lines_offsets[i] + background_textures[i].width() * photo_meter_per_pixel(i);
        }
        for (size_t i = 1; i < lines_of_stages.size(); i++) {
            for (Line& line : lines_of_stages[i]) {
                line.begin.x  += photo_lines_offsets[i];
                line.end.x    += photo_lines_offsets[i];
            }
        }
    }

    player.Init({20.0, Photo_world_Rect(0).y/3});
    player.transform_.velocity.x = 0;
    blackhole.Init(
        {Camera_world_Rect().x - Camera_world_Rect().x/7 + 5, Photo_world_Rect(0).y/7},
        Vec2{Camera_world_Rect().x/10, Photo_world_Rect(0).y/10} * 2,
        Camera_world_Rect()
    );
    game_start_stopwatch.start();
    player.stop_running();
}

static int32_t mod(int32_t x, int32_t p) {
    return (x % p + p) % p;
} 

void MainGame::update() {
    ClearPrint();
    player.update(effect);
    // 背景のループ処理
    {
        // 一番左端にある写真のX座標
        const double leftest_photo_x = photo_start_position(photo_index_count + 1);
        if (leftest_photo_x < player.transform_.position.x - Camera_world_Rect().x) {
            for (Line& line:lines_of_stages[mod(int32_t(photo_index_count), int32_t(photo_count()))]) {    
                line.begin.x    += photo_period_meter();
                line.end.x      += photo_period_meter();
            }
            photo_index_count++;
        }
    }
    // 衝突情報の更新
    {
        assert(collision_events.size() == lines_of_stages.size());
        for (size_t i = 0; i < lines_of_stages.size(); i++) {
            collision_events[i].clear();
            for (const auto [index, line] : IndexedRef(lines_of_stages[i])) {
                if (const auto collided_points = line.intersectsAt(player.collision_line())) {
                    collision_events[i].push_back({int(index), *collided_points});
                }
            }
        }
        if (const auto option_point = blackhole.collision_box().intersectsAt(player.collision_box())) {
            blackhole.shrink(player.transform_.velocity.lengthSq() * 1.2, (*option_point)[0]);
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

    // 開始処理
    if (gamestate == GS_Ready) {
        if (game_start_stopwatch.sF() >= 2.5) {
            blackhole.appear((game_start_stopwatch.sF() - 2.5) / 1.3);
        }
        if (game_start_stopwatch.s() >= 6) {
            bgm_game.play();
            gamestate = Playing;
            player.start_running();
            blackhole.is_ready = true;
            Console << U"a";
        }
    }

    if (is_game_end() and not game_end_stopwatch.isRunning()) { game_end_stopwatch.start(); }
    if (game_end_stopwatch.s() >= 6) {
        changeScene(U"Title", 3000);
    }
}

void MainGame::draw() const 
{
    {
        const Transformer2D centerized{Mat3x2::Translate(Scene::Center())};
        const Transformer2D scaled{Mat3x2::Scale(screen_pixel_per_meter())};
        const Transformer2D transformer{ Mat3x2::Translate(-scroll_offset()), TransformCursor::Yes };
        // 背景の描画
        if (background_textures.size() > 0) {
            for (
                size_t i = ((photo_index_count > 0) ? photo_index_count - 1 : 0);
                i <= photo_index_count + 2;
                i++
            ) {
                background_textures[i % photo_count()]
                    .resized(Photo_world_Rect(i % photo_count()))
                    .draw({photo_start_position(i), 0});
            }
        }
        effect.update();
        player.draw();
        
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
        switch (gamestate) {
            case GS_Ready:
            {
                if (game_start_stopwatch.sF() >= 4) {
                    RectF{{0, Scene::Height() * 0.4}, {Scene::Width(), Scene::Height() * 0.2}}.draw(ColorF{0, 0.5});
                    FontAsset(U"UIFont")(U"Ready...").drawAt(Scene::Center(), Palette::White);
                }
                break;
            }

            case Playing:
            {
                RectF{{0, 0}, {Scene::Width() * blackhole.destroyed_rate(), Scene::Height() / 20}}.draw(
                    HSV{20, 0.7, 0.70, 0.9}
                );
                break;
            }

            case Success:
            case Failed:
            {
                RectF{{0, Scene::Height() * 0.4}, {Scene::Width(), Scene::Height() * 0.2}}.draw(ColorF{0, 0.5});
                FontAsset(U"UIFont")(
                    (gamestate == Success) ? U"町を救った!" : U"Failed..."
                ).drawAt(Scene::Center(), Palette::White);
                break;
            }

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
