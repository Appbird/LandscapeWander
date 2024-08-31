#pragma once
#include <Siv3D.hpp>
#include "../App.hpp"

namespace JoyConController {

Polygon MakeGamePadPolygon()
{
	Polygon polygon = Ellipse{ 400, 480, 300, 440 }.asPolygon(64);
	polygon = Geometry2D::Subtract(polygon, Ellipse{ 400, 40, 220, 120 }.asPolygon(48)).front();
	polygon = Geometry2D::Subtract(polygon, Circle{ 400, 660, 240 }.asPolygon(48)).front();
	polygon = Geometry2D::Subtract(polygon, Rect{ 0, 540, 800, 60 }.asPolygon()).front();
	return polygon;
}


class Tester : public App::Scene {
public:

    const Ellipse buttonLB{ 210, 150, 50, 24 };
	const Ellipse buttonRB{ 590, 150, 50, 24 };
	const Polygon gamepadPolygon = MakeGamePadPolygon();
	const Circle logo{ 400, 250, 25 };
	const RectF leftTrigger{ 210, 16, 40, 100 };
	const RectF rightTrigger{ 550, 16, 40, 100 };
	const Circle leftThumb{ 230, 250, 35 };
	const Circle rightThumb{ 480, 350, 35 };
	const Circle dPad{ 320, 350, 40 };
	const Circle buttonA{ 570, 300, 20 };
	const Circle buttonB{ 620, 250, 20 };
	const Circle buttonX{ 520, 250, 20 };
	const Circle buttonY{ 570, 200, 20 };
	const Circle buttonView{ 330, 250, 15 };
	const Circle buttonMenu{ 470, 250, 15 };

    // プレイヤーインデックス (0 - 3)
	size_t playerIndex = 0;
	const Array<String> options;

	// デッドゾーンを有効にするか
	bool enableDeadZone = false;

	// 振動 (0.0 - 1.0)
	XInputVibration vibration;

    Tester(const InitData& init):
        IScene(init),
        options(Range(1, 4).map([](int32 i) {return U"{}P"_fmt(i); }))
    {
        const ColorF backgroundColor{ 0.6, 0.8, 0.7 };
	    Scene::SetBackground(backgroundColor);
    }
    void update() override {
		// 指定したプレイヤーインデックスの XInput コントローラを取得
		auto controller = XInput(playerIndex);

		// デッドゾーン
		if (enableDeadZone)
		{
			// それぞれデフォルト値を設定
			controller.setLeftTriggerDeadZone();
			controller.setRightTriggerDeadZone();
			controller.setLeftThumbDeadZone();
			controller.setRightThumbDeadZone();
		}
		else
		{
			// デッドゾーンを無効化
			controller.setLeftTriggerDeadZone(DeadZone{});
			controller.setRightTriggerDeadZone(DeadZone{});
			controller.setLeftThumbDeadZone(DeadZone{});
			controller.setRightThumbDeadZone(DeadZone{});
		}

		// 振動
		controller.setVibration(vibration);

		// L ボタン、R ボタン
		{
			buttonLB.draw(ColorF{ controller.buttonLB.pressed() ? 1.0 : 0.7 });
			buttonRB.draw(ColorF{ controller.buttonRB.pressed() ? 1.0 : 0.7 });
		}

		// 本体
		gamepadPolygon.draw(ColorF{ 0.9 });

		// Xbox ボタン
		{
			if (controller.isConnected())
			{
				Circle{ logo.center, 32 }
				.drawPie((-0.5_pi + 0.5_pi * controller.playerIndex), 0.5_pi, ColorF{ 0.6, 0.9, 0.3 });
			}

			logo.draw(ColorF{ 0.6 });
		}

		// 左トリガー
		{
			leftTrigger.draw(AlphaF(0.25));
			leftTrigger.stretched((controller.leftTrigger - 1.0) * leftTrigger.h, 0, 0, 0).draw();
		}

		// 右トリガー
		{
			rightTrigger.draw(AlphaF(0.25));
			rightTrigger.stretched((controller.rightTrigger - 1.0) * rightTrigger.h, 0, 0, 0).draw();
		}

		// 左スティック
		{
			leftThumb.draw(ColorF{ controller.buttonLThumb.pressed() ? 0.85 : 0.5 });
			Circle{ leftThumb.center + 25 * Vec2{ controller.leftThumbX, -controller.leftThumbY }, 20 }.draw();
		}

		// 右スティック
		{
			rightThumb.draw(ColorF{ controller.buttonRThumb.pressed() ? 0.85 : 0.5 });
			Circle{ rightThumb.center + 25 * Vec2{ controller.rightThumbX, -controller.rightThumbY }, 20 }.draw();
		}

		// 方向パッド
		{
			dPad.draw(ColorF{ 0.75 });
			Shape2D::Plus(dPad.r * 0.9, 25, dPad.center).draw(ColorF{ 0.5 });

			const Vec2 direction{
				controller.buttonRight.pressed() - controller.buttonLeft.pressed(),
				controller.buttonDown.pressed() - controller.buttonUp.pressed() };

			if (!direction.isZero())
			{
				Circle{ dPad.center + direction.withLength(25), 15 }.draw();
			}
		}

		// A, B, X, Y ボタン
		{
			buttonA.draw(ColorF{ 0.0, 1.0, 0.3, controller.buttonA.pressed() ? 1.0 : 0.3 });
			buttonB.draw(ColorF{ 1.0, 0.0, 0.3, controller.buttonB.pressed() ? 1.0 : 0.3 });
			buttonX.draw(ColorF{ 0.0, 0.3, 1.0, controller.buttonX.pressed() ? 1.0 : 0.3 });
			buttonY.draw(ColorF{ 1.0, 0.5, 0.0, controller.buttonY.pressed() ? 1.0 : 0.3 });
		}

		// View (Back), Menu (Start) ボタン 
		{
			buttonView.draw(ColorF(controller.buttonView.pressed() ? 1.0 : 0.7));
			buttonMenu.draw(ColorF(controller.buttonMenu.pressed() ? 1.0 : 0.7));
		}

		SimpleGUI::RadioButtons(playerIndex, options, Vec2{ 20, 20 });
		SimpleGUI::CheckBox(enableDeadZone, U"DeadZone", Vec2{ 320, 20 });
		SimpleGUI::Slider(U"leftMotor", vibration.leftMotor, Vec2{ 280, 420 }, 120, 120);
		SimpleGUI::Slider(U"rightMotor", vibration.rightMotor, Vec2{ 280, 460 }, 120, 120);
	}

    void draw() const override {

    }
    
};

}