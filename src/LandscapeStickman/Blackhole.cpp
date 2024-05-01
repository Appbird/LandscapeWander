#include "Blackhole.hpp"
#include "BlackHoleEffect.hpp"

namespace LandscapeStickman {

void Blackhole::update() {
    const double velocity_x = 20;
    if (is_ready) {
        if (not destroyed) {
            position.x += velocity_x * Scene::DeltaTime();
            time -= Scene::DeltaTime();
            if (received_energy > 1e-6) {
                // #FIXME フレームレートに依存する処理
                time += received_energy * 0.01;
                received_energy *= 0.99;
            }
            // リングエフェクト追加
            if (effect_interval.sF() > 3.0) {
                effect_interval.reset(); effect_interval.start();
                effect.add<BroadeningRing>(*this, collision_box().size, 3.5s);
            }
            // パーティクルエフェクト追加
            if (particle_interval.sF() > 0.06) {
                particle_interval.reset(); particle_interval.start();
                for (int i = 0; i < 2; i++) {
                    effect.add<HoleParticle>(position, 40, Vec2{velocity_x, 0});            
                }
            }
            if (time > max_time * 2) { destroyed = true; }
        } else {
            destroyed_count += Scene::DeltaTime();
            if (not eliminated and destroyed_count > 1) { 
                eliminated = true;
                effect.add<BubbleEffect>(position, 3);
            }
        }
    }

    this->w_sprite.Update();
}

void Blackhole::appear(double seq_time) {
    if (seq_time > 1) { return; }
    time = 60 + (15 - 60) * EaseOutBack(seq_time);
    if (not is_se_ring) {
        is_se_ring = true;
        AudioAsset(U"se/gravity").playOneShot();
        effect.add<BroadeningRing>(*this, Vec2{30, 30}, 1.0s);
    }

}

}