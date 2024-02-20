# include <Siv3D.hpp> // OpenSiv3D v0.6.11
# include "LandscapeStickman/MainGame.hpp"
# include "App.hpp"

void Main()
{
    App app;
    app.add<LandscapeStickman::MainGame>(U"LandscapeStickman/GameMain");
    while (System::Update()) {
        if (not app.update()) { continue; }
    }
}
