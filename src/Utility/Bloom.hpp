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

void Bloom(const BloomTextures& bloom_texures);