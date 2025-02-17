# include <Siv3D.hpp> // OpenSiv3D v0.6.11

# include "LandscapeStickman/MainGame.hpp"
# include "LandscapeStickman/InstructionScene.hpp"

# include "Firebase/Tester.hpp"
 
# include "LandscapeExtenders/MainGame.hpp"

# include "Joycon/tester.hpp"

# include "WalkDemo/MainGame.hpp"

# include "Title/MainMenu.hpp"

# include "Credits/Credit.hpp"

# include "AssetsRegister.hpp"

# include "App.hpp"
# include "Utility/Notificator.hpp"

void Main()
{
    RegisterAssets();
    Addon::Register<NotificationAddon>(U"NotificationAddon");
    NotificationAddon::SetLifeTime(3.0);

    Scene::SetResizeMode(ResizeMode::Keep);
    Window::SetStyle(WindowStyle::Sizable);

    App app;
    app.add<LandscapeStickman::MainGame>
        (U"LandscapeStickman/MainGame");
    app.add<LandscapeStickman::InstructionScene>
        (U"LandscapeStickman/InstructionScene");
    app.add<WalkDemo::MainGame>
        (U"WalkDemo/MainGame");
    app.add<Title::TitleScene>
        (U"Title");
    app.add<Credits::HomeScene>
        (U"Credit");
    app.add<Firebase::Tester>
        (U"FirebaseTester");
    app.add<LandscapeExtenders::MainGame>
        (U"LandscapeExtenders/MainGame");
    app.add<JoyConController::Tester>
        (U"JoyCon/Tester");
    
    app.init(U"LandscapeExtenders/MainGame", 1000);
    // app.init(U"JoyCon/Tester", 1000);
    while (System::Update()) {
        ClearPrint();
        if (not app.update()) { continue; }
    }
}
