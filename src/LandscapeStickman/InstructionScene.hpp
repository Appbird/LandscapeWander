#pragma once
#include <Siv3D.hpp>
#include "../App.hpp"

namespace LandscapeStickman {
class InstructionScene : public App::Scene {
private:
    Audio bgm_instruction;
    const Array<Texture> how_to_play;

    size_t page = 0;
public:
    InstructionScene(const InitData& init);
    void update() override;
    void draw() const override;
    
};
}
