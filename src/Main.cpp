# include <Siv3D.hpp> // OpenSiv3D v0.6.11
# include "LandscapeStickman/MainGame.hpp"
# include "LandscapeStickman/InstructionScene.hpp"
# include "WalkDemo/MainGame.hpp"
# include "Title/MainMenu.hpp"
# include "Credits/Credit.hpp"
# include "AssetsRegister.hpp"
# include "App.hpp"

void Main()
{
    RegisterAssets();

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
    
    app.init(U"Title", 1000);
    while (System::Update()) {
        if (not app.update()) { continue; }
    }
}
