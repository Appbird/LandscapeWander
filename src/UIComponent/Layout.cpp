# include "Layout.hpp"

/**
 * @brief padding pxぶんだけRectを膨張させる。
 * 
 * @param area 
 * @param padding 
 * @return Rect 
 */
Rect dilated(const Rect& area, const int padding){
    return Rect{
        area.pos - Point{padding, padding},
        area.size + 2 * Point{padding, padding}
    };
}
/**
 * @brief areaの左上点座標を(0,0), 左下点座標を(1,1)とした時にのclip領域を切り出したRectを返す。
 * 例えば、clipped(area, RectF{0, 0, 1, 0.5})は、areaの上半分を切り出した長方形になる。
 * @return Rect 
 */
Rect clipped(const Rect& area, const RectF& clip){
    return Rect{
        area.pos + (area.size * clip.pos).asPoint(),
        (area.size * clip.size).asPoint()
    };
}

Rect cliped_Y(const Rect& area, double top, double down){
    assert(InRange(top, 0.0, 1.0));
    assert(InRange(down, 0.0, 1.0));
    assert(top < down);
    return clipped(area, RectF{0, top, 1, down - top});
}
Rect cliped_X(const Rect& area, double right, double left){
    assert(InRange(right, 0.0, 1.0));
    assert(InRange(left, 0.0, 1.0));
    assert(right < left);

    return clipped(area, RectF{right, 0,left - right, 1});
}