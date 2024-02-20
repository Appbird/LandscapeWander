#pragma once
#include <Siv3D.hpp>
#include "../Utility/CollisionEvent.hpp"
#include "../Utility/bubble.hpp"
#include "../Utility/Spark.hpp"

namespace LandscapeStickman {

class Blackhole {
public:
    Vec2 camera_rect;
    Vec2 position;
    Vec2 basic_size;
    Effect effect;
    const double cover_length = 0;
    // ブラックホールが限界に達して画面を覆い尽くすまでの時間
    double max_time = 15;
    double time = max_time;
    double received_energy;

    Stopwatch effect_interval{StartImmediately::Yes};
    Stopwatch particle_interval{StartImmediately::Yes};

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
    void update();
    Vec2 current_size() const {
        double diameter = max_time/Max(time, 1e-6);
        if (destroyed) {
            diameter *= (1 - EaseInBack(destroyed_count));
        }
        if (eliminated) {
            diameter = 0;
        }
        return diameter * basic_size;
    }
    void draw() const {
        if (not eliminated) {
            Ellipse{collision_box()}
            .draw(HSV{
                20      + 30    * Periodic::Sine0_1(3s),
                0.7     + 0.1   * Periodic::Sine0_1(2s),
                0.70    + 0.05  * Periodic::Sine0_1(1s),
                0.9 
            })
            .stretched(-current_size().x/5)
            .drawFrame(current_size().x/10, HSV{40, 0.0, 1.0, 0.7});
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