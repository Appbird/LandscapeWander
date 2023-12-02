#pragma once
#include <Siv3D.hpp>

struct CollisionEvent {
    int32_t line_index;
    Vec2 collision_point;
};