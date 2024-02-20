#include "InstructionScene.hpp"

namespace LandscapeStickman {

InstructionScene::InstructionScene(const InitData& init) :
    IScene{ init }, 
    bgm_instruction(AudioAsset(U"bgm/instruction")),
    how_to_play{
        TextureAsset(U"instructions/page1"),
        TextureAsset(U"instructions/page2"),
        TextureAsset(U"instructions/page3")
    }
{
    Scene::SetBackground(ColorF{ 0.1, 0.1, 0.1 });
    bgm_instruction.play();
}

void InstructionScene::update() 
{
    if (KeyRight.up()) {
        page++;
        if (page == how_to_play.size()) {
            this->changeScene(U"LandscapeStickman/MainGame", 1000);
            bgm_instruction.stop();
        }    
    }
    if (KeyLeft.up()) { page--; }
    page = Clamp(int(page), 0, (int)(how_to_play.size() - 1));
}
void InstructionScene::draw() const
{
    const Rect instruction_area = Scene::Rect().stretched(-Scene::Width() / 15);
    FontAsset(U"UIFont")(U"[←]     [→]").draw(25, Arg::center = Scene::Rect().bottomCenter() - Vec2{ 0, Scene::Height() / 30 });
    how_to_play[page].resized(instruction_area.size).draw(instruction_area.tl());
}

}