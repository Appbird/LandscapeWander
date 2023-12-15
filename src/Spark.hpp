#pragma once
#include <Siv3D.hpp>
#include <chrono>

static double parabora(
    const double t,
    const double g,
    const double v0,
    const double x0
) {
    return t * t * g / 2 + t * v0 + x0;
}

struct SparkEffect : IEffect {
    const Vec2 center;
    const double size;
    const double lifespan;
    const double g;
    const Vec2 v0;
    /**
     * @brief 火花のエフェクトを表示する。size_の内部を運動するようにする。
     * initial_velocityで渡す長方形領域内からランダムに一点を取り、それを初速度とする。
     */
    SparkEffect(const Vec2& center_, const double& size_, const double gravity, const RectF& inital_velocity):
        center(center_),
        size(size_),
        lifespan(0.8),
        g(gravity),
        v0(RandomVec2(inital_velocity))
    {}
    bool update(double t) override
    {
        const double t1 = t * 0.6;
        const double t2 = t * 0.8;
        const double t3 = t;

        const double x1 = center.x + v0.x * t1;
        const double x2 = center.x + v0.x * t2;
        const double x3 = center.x + v0.x * t3;

        const double y1 = parabora(t1, g, v0.y, center.y);
        const double y2 = parabora(t2, g, v0.y, center.y);
        const double y3 = parabora(t3, g, v0.y, center.y);
        
        {
            const ScopedRenderStates2D blend{ BlendState::Additive };
            Bezier2{ {x1, y1}, {x2, y2}, {x3, y3} }.draw(LineStyle::RoundCap, size, ColorF{Palette::Orange, 0.8 * (1 - EaseInSine(t/lifespan))});
            Bezier2{ {x1, y1}, {x2, y2}, {x3, y3} }.draw(LineStyle::RoundCap, size/2, ColorF{Palette::Red, 1 - EaseInSine(t/lifespan)});
        }
        return t < lifespan;
    }
};
