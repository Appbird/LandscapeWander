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


Vec2 min(const Vec2& p1, const Vec2& p2) {
    return {
        std::min(p1.x, p2.x),
        std::min(p1.y, p2.y)
    };
}
Vec2 max(const Vec2& p1, const Vec2& p2) {
    return {
        std::max(p1.x, p2.x),
        std::max(p1.y, p2.y)
    };
}

RectF covered(const RectF& r1, const RectF& r2) {
    Vec2 topLeft = min(r1.tl(), r2.tl());
    Vec2 bottomRight = max(r1.br(), r2.br());
    return RectF{
        Arg::topLeft = topLeft,
        bottomRight - topLeft
    };
}

double gradarg_Line(const Line& line, const int32 direction) {
    assert(abs(direction) <= 1);
    if (line.end.x < line.begin.x) { return gradarg_Line(Line{line.end, line.begin}, direction); }
    return (direction * (line.end - line.begin)).getAngle();
}