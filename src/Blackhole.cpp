#include "Blackhole.hpp"
#include "BlackHoleEffect.hpp"

void Blackhole::update() {
    if (not destroyed) {
        position.x += 10 * Scene::DeltaTime();
        time -= Scene::DeltaTime();
    } else {
        destroyed_count += Scene::DeltaTime();
        if (not eliminated and destroyed_count > 1) { 
            eliminated = true;
            effect.add<BubbleEffect>(position, 3);
        }
    }
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
    if (particle_interval.sF() > 0.2) {
        particle_interval.reset(); particle_interval.start();
        effect.add<HoleParticle>(position, current_size().x * 1.5);
    }
    if (time > max_time * 2) { destroyed = true; }
}