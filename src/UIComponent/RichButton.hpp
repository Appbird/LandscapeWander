# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.11
# include "Layout.hpp"

class RichButton{
    HSV theme_color;
    String icon;
    String content;
    const Font icon_font{ FontMethod::MSDF, 40, Typeface::Icon_MaterialDesign };
    const Font text_font{ 40, FileSystem::GetFolderPath(SpecialFolder::UserFonts) + U"ロゴたいぷゴシック.otf" };
    Transition transition{ 0.2s, 0.2s };
    Rect base_whole;
    
    Rect actual_whole;
    Rect icon_place_rect;
    Rect text_place_rect;
    
public:
    bool selected = false;
    bool enabled = true;
    RichButton() { }
    RichButton(
        const String& arg_icon,
        const String& arg_content,
        const Rect& rect,
        bool arg_enabled = true,
        HSV color = HSV{30, 0.5, 0.8}
    ):
        theme_color(color),
        icon(arg_icon),
        content(arg_content),
        enabled(arg_enabled)
    {
        set_rect(rect);
    }

    void set_rect(const Rect& rect){
        base_whole = rect;
        actual_whole = base_whole;
    }
    bool leftClicked(){
        return base_whole.leftClicked() and enabled;
    }
    bool leftReleased(){
        return base_whole.leftReleased();
    }
    void update(){
        transition.update((base_whole.mouseOver() or selected) and enabled);
    }
    void render(){
        const double t = transition.value();
        const double e = EaseInOutExpo(t);
        const HSV background_color = 
            (selected) ? theme_color :
            (enabled) ? HSV{0, 0, 0.8} : HSV{0, 0, 0.4};
        actual_whole = Rect{base_whole.pos - Point{int(30 * e), 0}, base_whole.size };
        RoundRect{actual_whole.pos, actual_whole.size + Point{40, 0} , 5}.drawFrame(10, theme_color).draw(background_color);
        {
            RectSlicer layout_x { Rect{actual_whole.pos, actual_whole.size + Point{40, 0}}, RectSlicer::X_axis };
            icon_place_rect = layout_x.from(0.10).to(0.20);
            text_place_rect = layout_x.from(0.30).to(0.8);
        }
        icon_font(icon).draw((icon_place_rect.h * 0.8), Arg::center = icon_place_rect.center(), theme_color);
        text_font(content).draw((icon_place_rect.h * 0.25), Arg::leftCenter = text_place_rect.leftCenter(), ColorF{0.2});
    }
};