# include <Siv3D.hpp> // OpenSiv3D v0.6.11
# include "Animation.hpp"
# include "AnimationManager.hpp" 
# include "Player.hpp"
# include "CollisionEvent.hpp"
# include "image_process.hpp"
# include "Blackhole.hpp"

// #TODO 
    // ワームホールの中心に黒点
    // 数字を消す


// #TODO ディメンションホールの実装
    // 一定速度で左に行く
    // 楕円の描画(黒)
    // どんどん広がっていく
    // 主人公がぶつかると広がりが抑えられる。
    // 飛び状態の障害物にぶつかられるとディメンションホールが縮む
// #TODO 連続した長い写真に対応できるようにする。
// #TODO 障害物の定義
    // ディメンションホールから出てくる
    // 紫色の円
    // 速度を持つ
    // プレイヤーにぶつかると破壊
    // ジャンプ状態のプレイヤーにぶつかられると、プレイヤーの速度方向に飛んでいく
        // その状態でブラックホールにぶつかると、破壊される


Vec2 Clamp(const Vec2& value, const Vec2& min, const Vec2& max) {
    return Vec2{
        value.x, // Clamp(value.x, min.x, max.x),
        Clamp(value.y, min.y, max.y),
    };
}

// Glyph とエフェクトの関数を組み合わせてテキストを描画
void DrawText(const Font& font, double fontSize, const String& text, const Vec2& pos, const ColorF& color, double t,
	void f(const Vec2&, double, const Glyph&, const ColorF&, double), double characterPerSec)
{
	const double scale = (fontSize / font.fontSize());
	Vec2 penPos = pos;
	const ScopedCustomShader2D shader{ Font::GetPixelShader(font.method()) };

	for (auto&& [i, glyph] : Indexed(font.getGlyphs(text)))
	{
		if (glyph.codePoint == U'\n')
		{
			penPos.x = pos.x;
			penPos.y += (font.height() * scale);
			continue;
		}

		const double targetTime = (i * characterPerSec);

		if (t < targetTime)
		{
			break;
		}

		f(penPos, scale, glyph, color, (t - targetTime));

		penPos.x += (glyph.xAdvance * scale);
	}
}

// 文字が上からゆっくり降ってくる表現
void TextEffect1(const Vec2& penPos, double scale, const Glyph& glyph, const ColorF& color, double t)
{
	const double y = EaseInQuad(Saturate(1 - t / 0.3)) * -20.0;
	const double a = Min(t / 0.3, 1.0);
	glyph.texture.scaled(scale).draw(penPos + glyph.getOffset(scale) + Vec2{ 0, y }, ColorF{color, a});
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

enum GameState {
    G_Ready,
    Playing,
    Success,
    Failed
};
String get_text(int msg_code);
void Main()
{
    // 背景の色を設定する
	Scene::SetBackground(ColorF{ 0.1, 0.1, 0.1 });
    
    Window::SetStyle(WindowStyle::Sizable);

    Audio bgm_game{U"../assets/music/reflectable.mp3", Loop::Yes};
    Audio bgm_instruction{U"../assets/music/予兆.mp3", Loop::Yes};
    Audio se_bighit{U"../assets/se/hit.mp3"};
    assert(bgm_game);
    assert(bgm_instruction);

    GameState gamestate = G_Ready;

    BloomTextures bloom;
    Array<Texture> how_to_play{
        Texture{U"../assets/howto/page1.JPG" },
        Texture{U"../assets/howto/page2.JPG" },
        Texture{U"../assets/howto/page3.JPG" }
    };
    Array<Image> backgrounds{
        Image{U"../assets/test/ex1.png" },
        Image{U"../assets/test/ex2.png" },
        Image{U"../assets/test/ex3.png" },
        Image{U"../assets/test/ex4.png" },
        Image{U"../assets/test/ex5.png" },
    };
    Array<Vec2> backgrounds_offset(backgrounds.size());
    Array<Array<Line>> lines_of_stages;
    Array<Texture> background_textures;
    for (const Image& background : backgrounds){
        lines_of_stages.push_back(extract_stageline_from(background));
        background_textures.emplace_back(background);
    }

    Font UI_font{36, Typeface::Heavy};

    Vec2 position{1, 10};

    Effect effect;
    Array<Array<CollisionEvent>> collision_events(backgrounds.size());
    const double photo_world_width = 50;
    // 写真がワールド空間ではどれほどの大きさを持つか(mを単位とする。)
    Vec2 Photo_world_Rect = {photo_world_width, photo_world_width * backgrounds[0].size().y / backgrounds[0].size().x};
    
    const double camera_world_height = Photo_world_Rect.y;
    // カメラが写す画面の範囲(mを単位とする。)
    Vec2 Camera_world_Rect = {camera_world_height * Scene::Width() / Scene::Height(), camera_world_height};
    const double pixel_per_meter = Scene::Height() / camera_world_height;
    const double photo_meter_per_pixel = (photo_world_width / backgrounds[0].width());
    
    for (Array<Line>& lines_of_stage: lines_of_stages){
        for (Line& line : lines_of_stage) {
            line.begin  *= photo_meter_per_pixel;
            line.end    *= photo_meter_per_pixel;
        }
    }
    
    for (int i = 1; i < lines_of_stages.size(); i++) {
        for (Line& line : lines_of_stages[i]) {
            line.begin.x  += Photo_world_Rect.x * i;
            line.end.x    += Photo_world_Rect.x * i;
        }
        backgrounds_offset[i].x += Photo_world_Rect.x * i;
    }

    Player player{{3.0, Photo_world_Rect.y/3}};
    player.transform_.velocity.x = 20;
    Blackhole blackhole{
        {Camera_world_Rect.x - Camera_world_Rect.x/7, Photo_world_Rect.y/7},
        Vec2{Camera_world_Rect.x/10, Photo_world_Rect.y/5} * 2,
        Camera_world_Rect
    };
    int photo_passing_count = -2;
    int message_count = 0;

    Stopwatch ready_state{StartImmediately::Yes};
    Stopwatch game_start_stopwatch{StartImmediately::No};
    Stopwatch message_stopwatch{StartImmediately::Yes};
    int page = 0;
    bgm_instruction.play();
    while (System::Update()) {
        ClearPrint();
        switch (gamestate) {
            case G_Ready:
                {
                    const Rect instruction_area = Scene::Rect().stretched(-Scene::Width() / 15);
                    UI_font(U"[←]     [→]").draw(25, Arg::center = Scene::Rect().bottomCenter() - Vec2{ 0, Scene::Height() / 30 });
                    how_to_play[page].resized(instruction_area.size).draw(instruction_area.tl());
                    if (KeyRight.up()) {
                        page++;
                        if (page == how_to_play.size()) {
                            gamestate = Playing; 
                            game_start_stopwatch.start();
                            bgm_instruction.stop();
                            bgm_game.play();
                        }    
                    }
                    if (KeyLeft.up()) { page--; }
                    
                    page = Clamp(page, 0, (int)(how_to_play.size() - 1));
                    break;
                }
            case Playing:
            case Failed:
            case Success:
            {
                const ScopedColorMul2D colorMul{ ColorF{1, Min(1.0, game_start_stopwatch.sF())} };
                player.update(effect);
                // 背景ループなど
                {
                    // cameraの存在する場所
                    const int photo_index_world = (int)floor( (blackhole.position.x - (Camera_world_Rect.x/2 + blackhole.basic_size.x/2)) / photo_world_width ) - 2;
                    const int cycle_count = photo_index_world / backgrounds.size();
                    const int photo_index_camera = ((photo_index_world) % backgrounds.size()) + ((photo_index_world % backgrounds.size() >= 0) ? 0 : backgrounds.size());
                    
                    if (photo_passing_count < photo_index_world) {
                        if (photo_index_world >= 0) {
                            for (Line& line : lines_of_stages[photo_index_camera]) {
                                line.begin.x  += backgrounds.size() * Photo_world_Rect.x;
                                line.end.x    += backgrounds.size() * Photo_world_Rect.x;
                            }
                            backgrounds_offset[photo_index_camera].x += backgrounds.size() * Photo_world_Rect.x;
                        }
                        photo_passing_count = photo_index_world;
                    }
                }
                
                // 衝突情報の更新
                {
                    for (int i = 0; i < lines_of_stages.size(); i++) {
                        collision_events[i].clear();
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
                for (int i = 0; i < lines_of_stages.size(); i++) {
                    player.resolve_collision(collision_events[i], lines_of_stages[i]);
                }
                blackhole.update();

                const Vec2 scroll_offset = Clamp(
                    blackhole.position - Vec2{Camera_world_Rect.x/2 - blackhole.basic_size.x / 2, 0},
                    Vec2::Zero() + Camera_world_Rect/2,
                    Photo_world_Rect - Camera_world_Rect / 2
                );
                const double left_edge_x = scroll_offset.x - Camera_world_Rect.x/2;

                // クリア処理
                if (blackhole.destroyed and player.should_running) {
                    player.stop_running();
                    player.controllable_state = false;
                    gamestate = Success;
                }
                // ミス処理
                if (
                    blackhole.is_covering_all_region(left_edge_x) and player.should_running or 
                    player.collision_box().topY() > Camera_world_Rect.y + 2 and player.should_running
                ) {
                    player.stop_running();
                    player.controllable_state = false;
                    gamestate = Failed;
                }
                
                
                {
                    const Transformer2D centerized{Mat3x2::Translate(Scene::Center())};
                    const Transformer2D scaled{Mat3x2::Scale(pixel_per_meter)};
                    const Transformer2D transformer{ Mat3x2::Translate(-scroll_offset), TransformCursor::Yes };
                    for (int i = 0; i < background_textures.size(); i++) {
                        background_textures[i].resized(Photo_world_Rect).draw(backgrounds_offset[i]);
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
                        const ScopedRenderTarget2D bloom_target{bloom.blur1.clear(ColorF{0})};
                        const ScopedColorMul2D colorMul{ ColorF{1, 0.2*Periodic::Jump0_1(1.0s) + 0.8} };
                        player.draw();
                    }
                }
                
                Bloom(bloom);

                // UI関連
                {
                    if (gamestate != Playing) {
                        // リザルト表示
                        RectF{{0, Scene::Height() * 0.4}, {Scene::Width(), Scene::Height() * 0.2}}.draw(ColorF{0, 0.5});
                        UI_font(
                            (gamestate == Success) ? U"街を救った!" : U"Failed..."
                        ).drawAt(Scene::Center(), Palette::White);
                    } else {
                        RectF{{0, 0}, {Scene::Width() * blackhole.destroyed_rate(), Scene::Height() / 20}}.draw(
                                HSV{20, 0.7, 0.70, 0.9}
                        );
                    }
                }
            }
            
        }
        // 会話ログ
        const RectF dialog_area{
            {0, Scene::Height() * 0.8},
            {Scene::Width(), Scene::Height() * 0.2}
        };
        
        
        if (message_stopwatch.s() > 4.0 * (message_count + 1)) {
            message_count++;
        }
        const String text = get_text(message_count);
        if (not text.empty()) {
            dialog_area.draw(ColorF{0, 0.4});
            UI_font(U"司令部").draw(24, Arg::topLeft = dialog_area.stretched(-10).tl(), ColorF{200});
            UI_font(text).draw(18, Arg::center = dialog_area.stretched(-Scene::Width()/8).center(),  ColorF{200});   
        }
        
    }    
}

String get_text(int msg_code) {
    switch(msg_code) {
        default:
            return U"";
    }
    return U"";
}