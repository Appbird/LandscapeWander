#pragma once
#include <Siv3D.hpp>
#include "../App.hpp"

namespace JoyConController {

class Tester : public App::Scene {
public:

    Effect effect;
	Vec2 left{ 640 - 100, 100 }, right{ 640 + 100, 100 };
	double angle = 0_deg;
	double scale = 400.0;
	bool covered = true;

    Tester(const InitData& init):
        IScene(init)
    {
        Scene::SetBackground(ColorF{ 0.9 });
        Scene::SetResizeMode(ResizeMode::Virtual);
	    Window::Resize(1280, 720);
    }
    void update() override {
        Circle{ Vec2{ 640 - 300, 450 }, scale / 2 }.drawFrame(scale * 0.1);
		Circle{ Vec2{ 640 + 300, 450 }, scale / 2 }.drawFrame(scale * 0.1);
        
		// Joy-Con (L) を取得
		if (const auto joy = JoyConL(0))
		{
			joy.drawAt(Vec2{ 640 - 300, 450 }, scale, -90_deg - angle, covered);

			if (auto d = joy.povD8())
			{
				left += Circular{ 4, *d * 45_deg };
			}

			if (joy.button2.down())
			{
				effect.add([center = left](double t) {
					Circle{ center, 20 + t * 200 }.drawFrame(10, 0, AlphaF(1.0 - t));
					return t < 1.0;
					});
			}
		}

		// Joy-Con (R) を取得
		if (const auto joy = JoyConR(0))
		{
			joy.drawAt(Vec2{ 640 + 300, 450 }, scale, 90_deg + angle, covered);

			if (auto d = joy.povD8())
			{
				right += Circular{ 4, *d * 45_deg };
			}

			if (joy.button2.down())
			{
				effect.add([center = right](double t) {
					Circle{ center, 20 + t * 200 }.drawFrame(10, 0, AlphaF(1.0 - t));
					return t < 1.0;
					});
			}
		}
        

		Circle{ left, 30 }.draw(ColorF{ 0.0, 0.75, 0.9 });
		Circle{ right, 30 }.draw(ColorF{ 1.0, 0.4, 0.3 });
		effect.update();

		SimpleGUI::Slider(U"Rotation: ", angle, -180_deg, 180_deg, Vec2{ 20, 20 }, 120, 200);
		SimpleGUI::Slider(U"Scale: ", scale, 100.0, 600.0, Vec2{ 20, 60 }, 120, 200);
		SimpleGUI::CheckBox(covered, U"Covered", Vec2{ 20, 100 });
    }

    void draw() const override {

    }
    
};

}