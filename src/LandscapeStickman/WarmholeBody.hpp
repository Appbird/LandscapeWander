#pragma once
#include <Siv3D.hpp>
class WarmholeSprite {
public:
    VertexShader vs;
    PixelShader ps;
    struct SoftShape {
        float g_t;
    };
    ConstantBuffer<SoftShape> softshape;
    WarmholeSprite ():
        vs(
            GLSL{ U"./assets/shader/blackhole.vert", {
                { U"VSConstants2D", 0 },
                { U"SoftShape", 1 }
            }}
        ),
        ps(
            GLSL{
                U"./assets/shader/blackhole.frag", {
                    {U"PSConstants2D", 0}
                }
            }
        )
    {
        assert(vs);
        assert(ps);
    }
    void Update() { softshape->g_t = Scene::Time(); }
    void Draw(
        const Vec2 position,
        const double radius
    ) const {
        Graphics2D::SetVSConstantBuffer(1, softshape);
        const Transformer2D tf1{Mat3x2::Translate(position)};
        const Transformer2D tf2{Mat3x2::Scale(radius)};
        const ScopedCustomShader2D shaders{vs, ps};
        Graphics2D::DrawTriangles(360);
    }
};
