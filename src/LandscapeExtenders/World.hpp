#pragma once
#include <Siv3D.hpp>
#include "EdgeDetectedStage.hpp"
#include "../Utility/numeric.hpp"

namespace LandscapeExtenders {

using std::shared_ptr;

struct CollisionTicket {
    shared_ptr<Line> collided;
    Vec2 collided_point;
};

class Hittable {
    virtual bool hit(
        const Line& object,
        Array<CollisionTicket>& tickets
    ) const = 0;
};

struct PhotoStage : Hittable {
    PhotoStage(const EdgeDetectedStage& stage) : edge_detected_stage(stage) {}
    bool includes(Vec2 pos) const {
        return edge_detected_stage.info.includes(pos);
    }
    RectF bounding_box() const {
        return edge_detected_stage.info.area;
    }
    void draw() const {
        edge_detected_stage.landscape
            .resized(edge_detected_stage.info.area.size)
            .draw(edge_detected_stage.info.area.tl());
    }
    void draw_lines() const {
        for (const auto& line : edge_detected_stage.terrain) {
            // #TODO ラインの描画方法について考える。
            line->draw(0.2,
                HSV{120, 0.4, 1, 0.7 + 0.2 * Periodic::Sine0_1(2s)},
                HSV{150, 0.4, 1, 0.7 + 0.2 * Periodic::Sine0_1(2s)}
            );
        }
    }
    bool hit(
        const Line& object,
        Array<CollisionTicket>& tickets
    ) const {
        bool hitted = false;
        for (const auto& line : edge_detected_stage.terrain) {
            if (const auto collided_point = line->intersectsAt(object)) {
                tickets.push_back({line, *collided_point});
                hitted = true;
            }
        }
        return hitted;
    }
private:
    EdgeDetectedStage edge_detected_stage;
};

struct World : Hittable {
    const String empty_str = U"";
    void initialize();
    /** `pos`の位置する地点を返す。 もしどこのステージにも位置していなければ空文字列を返す。*/
    const String& where(const Vec2 pos) const {
        for (const auto& [key, stage] : stages) {
            if (stage.includes(pos)) { return key; }
        }
        return empty_str;
    }
    void draw(const RectF& visible) const {
        for (const auto& [key, stage] : stages) {
            // 映るものだけドローコールを呼び出す。
            if (stage.bounding_box().intersects(visible)) { stage.draw(); }
        }
    }
    void draw_lines(const RectF& visible) const {
        for (const auto& [key, stage] : stages) {
            // 映るものだけドローコールを呼び出す。
            if (stage.bounding_box().intersects(visible)) { stage.draw_lines(); }
        }
    }

    bool hit(
        const Line& object,
        Array<CollisionTicket>& tickets
    ) const {
        bool hitted = false;
        Array<CollisionTicket> events;
        const String& p1 = where(object.begin);
        if (not p1.empty()) { hitted |= stages.at(p1).hit(object, tickets); }
        const String& p2 = where(object.end);
        if (not p1.empty()) { hitted |= stages.at(p2).hit(object, tickets); }

        return hitted;
    }

    RectF bounding_box() {
        if (stages.size() == 0) { return RectF::Empty(); }
        RectF result = stages.begin()->second.bounding_box();
        for (const auto& [key, stage] : stages) {
            result = covered(result, stage.bounding_box());
        }
        return result;
    }
    private:
        HashTable<String, PhotoStage> stages;
};

}