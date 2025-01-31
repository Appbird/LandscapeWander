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

double atan2(const Vec2 v) {
    return std::atan2(v.y, v.x);
}

double gradarg_Line(const Line& line, const int32 direction) {
    assert(abs(direction) <= 1);
    if (line.end.x < line.begin.x) { return gradarg_Line(Line{line.end, line.begin}, direction); }
    Vec2 move_dir = direction * (line.end - line.begin);
    if (move_dir.isZero()) { return 0; }
    move_dir.x = abs(move_dir.x);
    return atan2(move_dir);
}


double segment_distance(double l1, double r1, double l2, double r2) { 
    if (r1 < l1) { return segment_distance(r1, l1, l2, r2); }
    if (r2 < l2) { return segment_distance(l1, r1, r2, l2); }
    return std::max(0.0, std::max(l1, l2) - std::min(r2, r1));
}

double lerp(const double x, const double y0, const double x0) {
    assert(0 <= x and x <= 1);
    return y0 / x0 * (x - x0) + y0;
}