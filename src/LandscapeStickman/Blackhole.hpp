#pragma once
#include <Siv3D.hpp>
#include "../Utility/CollisionEvent.hpp"
#include "../Utility/bubble.hpp"
#include "../Utility/Spark.hpp"
#include "./WarmholeBody.hpp"

namespace LandscapeStickman {

class Blackhole {
public:
    Vec2 camera_rect;
    Vec2 position;
    Vec2 basic_size;
    Effect effect;
    WarmholeSprite w_sprite;

    
    const double cover_length = 0;
    // ブラックホールが限界に達して画面を覆い尽くすまでの時間
    double max_time = 15;
    double time = 500;
    double received_energy = 0;

    Stopwatch effect_interval{StartImmediately::Yes};
    Stopwatch particle_interval{StartImmediately::Yes};

    bool is_ready = false;
    bool is_se_ring = false;
    bool is_first_ring_appear = false;
    bool destroyed = false;
    double destroyed_count = 0;
    bool eliminated = false;
    
    void Init(
        Vec2 position_,
        Vec2 basic_size_,
        Vec2 camera_rect_
    ){
        position = (position_);
        basic_size = (basic_size_);
        camera_rect = (camera_rect_);
    }
    void appear(double seq_time);
    void update();
    Vec2 current_size() const {
        double diameter = max_time/Max(time, 1e-3);
        if (destroyed) {
            diameter *= (1 - EaseInBack(destroyed_count));
        }
        if (eliminated) {
            diameter = 0;
        }
        return diameter * basic_size;
    }
    void draw() const {
        if (is_se_ring and not eliminated) {
            const RectF cb = collision_box();
            w_sprite.Draw(cb.center(), cb.w / 1.5);
        }
        effect.update();
    }
    RectF collision_box() const {
        return RectF{position - current_size(), current_size()*2};
    }
    void shrink(double energy, const Vec2 touched_point) {
        for (int i = 0; i < 40; i++) {
            effect.add<SparkEffect>((touched_point + position)/2, 0.10, 9.8*3, RectF{{-15, -10}, {40, 20}});
        }
        received_energy += energy / 100;
    }
    double destroyed_rate() const {
        return (time) / (2 * max_time);
    }
    bool is_covering_all_region(const double leftedge) const {
        return collision_box().leftX() < leftedge;
    }
};

};