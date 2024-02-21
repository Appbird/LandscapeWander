# pragma once
# include <Siv3D.hpp>
# include <utility>

/**
 * @brief padding pxぶんだけRectを膨張させる。
 * 
 * @param area 
 * @param padding 
 * @return Rect 
 */
Rect dilated(const Rect& area, const int padding);
/**
 * @brief areaの左上点座標を(0,0), 左下点座標を(1,1)とした時にのclip領域を切り出したRectを返す。
 * 例えば、clipped(area, RectF{0, 0, 1, 0.5})は、areaの上半分を切り出した長方形になる。
 * @return Rect 
 */
Rect clipped(const Rect& area, const RectF& clip);
Rect cliped_Y(const Rect& area, double top, double down);
Rect cliped_X(const Rect& area, double right, double left);


class RectSlicer{
    public:
        enum Axis{
            X_axis, Y_axis
        };
    private:
        Axis direction;
        Rect whole;
        double start = 0;
    public:
        RectSlicer(const Rect& arg_whole, Axis arg_direction):
            direction(arg_direction),
            whole(arg_whole)
        {}
        Rect to(const double slice_at){
            assert(start != 1.0);
            assert(start < slice_at);
            assert(0 <= slice_at and slice_at <= 1);
            const Rect result =
                (direction == Y_axis)
                    ? Rect{
                        whole.pos + Point{0, int(whole.size.y * start)},
                        Size{whole.size.x, int(whole.size.y * (slice_at - start))}
                    }
                    : Rect{
                        whole.pos + Point{int(whole.size.x * start), 0},
                        Size{int(whole.size.x * (slice_at - start)), whole.size.y}
                    };
            start = slice_at;
            return result;
        }
        RectSlicer slice(const double slice_at, const Axis next_axis){
            return {to(slice_at), next_axis};
        }
        std::pair<RectSlicer, RectSlicer> devide_at(const double at, const Axis next_axis){
            assert(start < at); assert(at < 1.0);
            return {slice(at, next_axis), slice(1.0, next_axis)};
        }
        RectSlicer& from(const double from){
            assert(start < from);
            start = from;
            return *this;
        }
        Rect rect(){
            return whole;
        }
        
};