#include <Siv3D.hpp>

Vec2 ClampY(const Vec2& value, const Vec2& min, const Vec2& max) {
    return Vec2{
        value.x, // Clamp(value.x, min.x, max.x),
        Clamp(value.y, min.y, max.y),
    };
}

Vec2 ClampXY(const Vec2& value, const Vec2& min, const Vec2& max) {
    return Vec2{
        Clamp(value.x, min.x, max.x),
        Clamp(value.y, min.y, max.y),
    };
}
