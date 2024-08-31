#pragma once
#include <Siv3D.hpp>

Vec2 ClampY(const Vec2& value, const Vec2& min, const Vec2& max);
Vec2 ClampXY(const Vec2& value, const Vec2& min, const Vec2& max);

Vec2 min(const Vec2& p1, const Vec2& p2);
Vec2 max(const Vec2& p1, const Vec2& p2);

RectF covered(const RectF& r1, const RectF& r2);

/**
 * @brief プレイヤーが向かおうとしている方向`direction`に対して、どれだけの傾斜があるかを返します。
 * 
 */
double gradarg_Line(const Line& line, const int32 direction);