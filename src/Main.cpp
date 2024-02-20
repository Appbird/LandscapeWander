# include <Siv3D.hpp> // OpenSiv3D v0.6.11
# include "LandscapeStickman/MainGame.hpp"
# include "LandscapeStickman/InstructionScene.hpp"
# include "AssetsRegister.hpp"
# include "App.hpp"

void Main()
{
    RegisterAssets();

    App app;
    app.add<LandscapeStickman::MainGame>(U"LandscapeStickman/GameMain");
    app.add<LandscapeStickman::InstructionScene>(U"LandscapeStickman/InstructionScene");
    app.init(U"LandscapeStickman/InstructionScene");
    while (System::Update()) {
        if (not app.update()) { continue; }
    }
}
