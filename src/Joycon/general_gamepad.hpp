#pragma once
#include <Siv3D.hpp>
#include "../App.hpp"

namespace JoyConController {
class Tester : public App::Scene {
public:

    const Array<String> indices;
	    // ゲームパッドのプレイヤーインデックス
	    size_t playerIndex = 0;

    Tester(const InitData& init):
        IScene(init),
        indices(Range(0, (Gamepad.MaxPlayerCount - 1)).map(Format))
    {
    }
    void update() override {
        ClearPrint();

		if (const auto gamepad = Gamepad(playerIndex)) // 接続されていたら
		{
			const auto& info = gamepad.getInfo();

			Print << U"{} (VID: {}, PID: {})"_fmt(info.name, info.vendorID, info.productID);

			for (auto [i, button] : Indexed(gamepad.buttons))
			{
				Print << U"button{}: {}"_fmt(i, button.pressed());
			}

			for (auto [i, axe] : Indexed(gamepad.axes))
			{
				Print << U"axe{}: {}"_fmt(i, axe);
			}

			Print << U"POV: " << gamepad.povD8();
		}

		SimpleGUI::RadioButtons(playerIndex, indices, Vec2{ 500, 20 });
	
    }

    void draw() const override {

    }
    
};

}