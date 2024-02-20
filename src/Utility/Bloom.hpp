# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.11
struct BloomTextures {
    const RenderTexture blur1;
    const RenderTexture internal1;
    const RenderTexture blur4;
    const RenderTexture internal4;
    const RenderTexture blur8;
    const RenderTexture internal8;
    BloomTextures():
        blur1{Scene::Size()},
        internal1{Scene::Size()},
        blur4{Scene::Size() / 4},
        internal4{Scene::Size() / 4},
        blur8{Scene::Size() / 8},
        internal8{Scene::Size() / 8}
    {}      
};

void Bloom(BloomTextures& bloom_texures) {
    {
        Shader::GaussianBlur(bloom_texures.blur1, bloom_texures.internal1, bloom_texures.blur1);
        
        Shader::Downsample(bloom_texures.blur1, bloom_texures.blur4);
        Shader::GaussianBlur(bloom_texures.blur4, bloom_texures.internal4, bloom_texures.blur4);

        Shader::Downsample(bloom_texures.blur4, bloom_texures.blur8);
        Shader::GaussianBlur(bloom_texures.blur8, bloom_texures.internal8, bloom_texures.blur8);
    }
    {
        const ScopedRenderStates2D blend{BlendState::Additive};
        bloom_texures.blur4.resized(Scene::Size()).draw(ColorF{0.4});
        bloom_texures.blur8.resized(Scene::Size()).draw(ColorF{0.5});
    }

}