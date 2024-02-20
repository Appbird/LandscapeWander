#pragma once
#include <Siv3D.hpp>

using App = SceneManager<String>;

Vec2 Clamp(const Vec2& value, const Vec2& min, const Vec2& max) {
    return Vec2{
        value.x, // Clamp(value.x, min.x, max.x),
        Clamp(value.y, min.y, max.y),
    };
}