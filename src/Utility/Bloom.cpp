# include "./Bloom.hpp"

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