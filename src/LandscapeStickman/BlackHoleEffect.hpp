#pragma once
#include <Siv3D.hpp>
#include <chrono>
#include "Blackhole.hpp"

namespace LandscapeStickman {

struct BroadeningRing : IEffect {
    const Blackhole& blackhole;
    const double inital_width;
    const Vec2 inital_size;
    const double lifespan;
    /**
     * @brief ブラックホールの吸い込み円のエフェクトを表示する。size_を覆う領域において、初期状態で見えないぎりぎりの楕円の大きさから始める。
     */
    BroadeningRing(
        const Blackhole& blackhole_,
        const Vec2& size_,
        std::chrono::duration<long double> lifespan_
    ):
        blackhole(blackhole_),
        inital_width(size_.length()/ 20),
        inital_size(size_),
        lifespan(lifespan_.count())
    {}
    bool update(double t) override
    {
        const Vec2 size = EaseInQuad(t/lifespan) * inital_size;
        const double p = (1 - EaseInSine(t/lifespan));
        const double frame_width = inital_width * p;
        Ellipse(blackhole.position, size).drawFrame(frame_width, ColorF{0, p});
        if (size.lengthSq() > (2*frame_width)*(2*frame_width)) {
            Ellipse(blackhole.position, size).stretched(-frame_width).drawFrame(frame_width, ColorF{0, 0.6 * p});
            Ellipse(blackhole.position, size).stretched(-2*frame_width).drawFrame(frame_width, ColorF{0, 0.4 * p});
        }
        return t < lifespan;
    }
};

static Vec2 spiral(const double t, const double omega, const double delta) {
    return Vec2{
        exp(t) * cos(omega * t + delta) - cos(delta),
        exp(t) * sin(omega * t + delta) - sin(delta)
    } / (Math::E - 1);
}

struct HoleParticle : IEffect {
    const Vec2 center;
    const double area_radius;
    const double particle_radius;
    const double inital_phase;
    const double opacity;
    const double omega;
    const double lifespan;
    const Vec2 velocity;
    /**
     * @brief ブラックホールの吸い込みパーティクルに対応する。size_を覆う領域において、初期状態で見えないぎりぎりの楕円の大きさから始める。
     */
    HoleParticle(const Vec2& center_, const double area_radius, const Vec2& velocity_):
        center(center_),
        area_radius(area_radius),
        particle_radius (Random(area_radius/200, area_radius/50)),
        inital_phase    (Random(0.0, 2*Math::Pi)),
        opacity         (Random(0.3, 0.8)),
        omega           (Random(0.0, Math::Pi)),
        lifespan        (Random(3.0, 8.0)),
        velocity        (velocity_)
    {}
    bool update(double t) override
    {
        const ScopedRenderStates2D blend{ BlendState::Additive };
        const double p = EaseInOutSine(t / lifespan);
        const Vec2 point = 
            center + area_radius * spiral(p, omega, inital_phase) + velocity * t;
        Circle(point, particle_radius).draw(ColorF{1.0, 0.6314, 0.1098, opacity * (1-p)});
        Circle(point, particle_radius*0.7).draw(ColorF{1.0, 0.6314, 0.1098, opacity * (1-p)});
        return t < lifespan;
    }
};

}