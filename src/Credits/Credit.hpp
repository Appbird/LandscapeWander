#pragma once
#include <Siv3D.hpp>
#include "../App.hpp"
#include "../UIComponent/RichButton.hpp"
#include "../UIComponent/Tile.hpp"

namespace Credits {

class HomeScene : public App::Scene {
    private:
        size_t selected_package_index = 0;
        Rect center_space;
        String credits_text;

        Transition playbutton_transition{ 0.1s, 0.1s };
    public:
        HomeScene(const InitData& init) : IScene{init}
        {
            TextReader reader{ U"assets/credits.md" };
            assert(reader);
            Scene::SetBackground(HSV{ 200, 0.15, 0.95 });
            center_space = Scene::Rect().stretched(-20);
            credits_text = reader.readAll();
        }
        void update() override
        {
            if (KeyB.up() or MouseL.up()) { changeScene(U"Title", 3000); }
        }
        void draw() const override
        {
            RoundRect{center_space, 5}.draw(ColorF{1.0, 1.0, 1.0, 1.0});
            FontAsset(U"UIFont")(credits_text).draw(20, center_space.stretched(-5), Palette::Black);
        }
        
};

}
