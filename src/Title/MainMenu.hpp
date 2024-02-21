#pragma once
#include <Siv3D.hpp>
#include "../App.hpp"
#include "../UIComponent/RichButton.hpp"
#include "../UIComponent/Tile.hpp"

namespace Title {
struct Package {
    Icon   icon;
    String tile_name;
    String title;
    String next_scene;
    String description;
    String how_to_play;
    TileButton::Palette palette;
};

class TitleScene : public App::Scene {
    private:
        size_t selected_package_index = 0;
        Rect header_space;
        Rect center_space;
        Rect bottom_space;
            Rect description_area;
            Rect author_area;
            Rect how_to_play;
            Rect play_button;
        
        Array<Package> titles;
        Array<Rect> buttons_regions;
        Array<TileButton> tilebuttons;

        Transition playbutton_transition{ 0.1s, 0.1s };
    public:
        TitleScene(const InitData& init) : IScene{init}
        {
            Scene::SetBackground(HSV{ 200, 0.15, 0.95 });
            titles = {
                { 
                    0xF02B4_icon,
                    U"Stickman",
                    U"Landscape Stickman",
                    U"LandscapeStickman/InstructionScene",
                    U"ワームホールがあなたの街に現れた！\n棒人間ヒーローが街を救う！",
                    U"[スペース]: チャージ, [^][->]: ジャンプ方向調節",
                    { HSV{ 75, 0.75, 0.75 }, Palette::White, ColorF{ 1.0, 0.4 }, HSV{ 75, 0.5, 1 } }
                },
                { 
                    0xF062E_icon,
                    U"Wander",
                    U"Landscape Wander",
                    U"WalkDemo/MainGame",
                    U"Landscape Wanderの技術デモ",
                    U"[スペース]: ジャンプ, [<-][->]: ダッシュ",
                    { HSV{ 125, 0.75, 0.75 }, Palette::White, ColorF{ 1.0, 0.4 }, HSV{ 125, 0.5, 1 } }
                },
                { 
                    0xF062E_icon,
                    U"Credits",
                    U"クレジット",
                    U"LandscapeWander/InstructionScene",
                    U"この作品の制作に用いたもののリスト。",
                    U"上下キー: スクロール",
                    { HSV{ 175, 0.75, 0.75 }, Palette::White, ColorF{ 1.0, 0.4 }, HSV{ 175, 0.5, 1 } }
                },
            };
            assert(titles.size() == 3);
            buttons_regions.resize(3);
            {
                RectSlicer rs{cliped_X(Scene::Rect(), 0.05, 0.95), RectSlicer::Axis::Y_axis};
                header_space = rs.to(0.15);
                center_space = rs.to(0.5);
                {
                    RectSlicer rs{center_space, RectSlicer::Axis::X_axis};
                    buttons_regions[0] = rs.to(0.333).stretched(-10);
                    buttons_regions[1] = rs.to(0.666).stretched(-10);
                    buttons_regions[2] = rs.to(0.999).stretched(-10);
                }
                bottom_space = rs.to(0.9);
                {
                    RectSlicer rs{bottom_space, RectSlicer::Axis::X_axis};
                    Rect left_area = rs.to(0.75);
                    {
                        RectSlicer rs{left_area, RectSlicer::Axis::Y_axis};
                        description_area    = rs.to(0.6).stretched(-5);
                        author_area         = rs.to(1.0).stretched(-5);
                    }
                    Rect right_area = rs.to(1.0);
                    {
                        RectSlicer rs{right_area, RectSlicer::Axis::Y_axis};
                        play_button = rs.to(0.3).stretched(-5);
                        how_to_play = rs.to(1.0).stretched(-5);
                    }
                }
            }
            
            tilebuttons.resize(3);
            for (int i = 0; i < 3; i++) {
                tilebuttons[i] = TileButton{
                    titles[i].icon,
                    int32(double(buttons_regions[i].w) * 0.6),
                    FontAsset(U"UIFont"),
                    double(int32(double(buttons_regions[i].w)/ 10.0)),
                    titles[i].tile_name,
                    buttons_regions[i],
                    titles[i].palette
                };
            }
        }
        void update() override
        {
            // #TODO 続きここから
            //# TODO あと、Xcodeに追加し忘れのコードがないか確認。
                // 多分Layout付近は全然やっていない。
            for (size_t i = 0; i < tilebuttons.size(); i++)
            {
                if (buttons_regions[i].mouseOver()) {
                    selected_package_index = i;    
                }
                tilebuttons[i].is_selected = i == selected_package_index;
                if (tilebuttons[i].update()) {
                    changeScene(titles[i].next_scene);
                }
            }

            playbutton_transition.update(play_button.mouseOver());
            if (KeyLeft.up()) { selected_package_index = Clamp(int32_t(selected_package_index) - 1, 0, 3 - 1); }
            if (KeyRight.up()) { selected_package_index = Clamp(int32_t(selected_package_index) + 1, 0, 3 - 1); }
            if (KeySpace.up() or play_button.leftClicked()) { changeScene(titles[selected_package_index].next_scene); }
        }
        void draw() const override
        {
            for (const auto& button : tilebuttons)
            {
                button.draw();
            }
            HSV description_font_color = HSV{200, 0.1, 0.05};
            
            RoundRect{description_area, 5}.draw(ColorF{1});
            FontAsset(U"UIFont")(titles[selected_package_index].title).draw(25, Arg::topLeft = description_area.stretched(-5).tl(), description_font_color);
            FontAsset(U"UIFont")(titles[selected_package_index].description).draw(16, Arg::topLeft = description_area.stretched(-5).leftCenter(), description_font_color);
            
            RoundRect{author_area, 5}.draw(ColorF{1});
            FontAsset(U"UIFont")(titles[selected_package_index].how_to_play).draw(16, Arg::center = author_area.stretched(-5).center(), description_font_color);
            
            RoundRect{how_to_play, 5}.draw(ColorF{1});

            {
                const double t = playbutton_transition.value();
                RoundRect{play_button.stretched(3 * (2*EaseInOutExpo(t) - 1)), 5}.draw(HSV{240, 0.80, 0.80});
                FontAsset(U"UIFont")(U"スタート").draw(18, Arg::center = play_button.stretched(-5).center(), Palette::White);
            }
        }
        
};

}
