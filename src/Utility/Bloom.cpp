# include "./Bloom.hpp"


void Bloom(const BloomTextures& bt, bool additive) {
    Shader::GaussianBlur(bt.blur1, bt.internal1, bt.blur1);
    
    Shader::Downsample(bt.blur1, bt.blur4);
    Shader::GaussianBlur(bt.blur4, bt.internal4, bt.blur4);

    Shader::Downsample(bt.blur4, bt.blur8);
    Shader::GaussianBlur(bt.blur8, bt.internal8, bt.blur8);
    
    {
        const ScopedRenderStates2D blend{(additive) ? BlendState::Additive : BlendState::Default2D};
        bt.blur4.resized(Scene::Size()).draw(ColorF{0.4});
        bt.blur8.resized(Scene::Size()).draw(ColorF{0.5});
    }

}