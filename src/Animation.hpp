#pragma once
#include <Siv3D.hpp>
#include "Interval.hpp"

/// @brief (ループする)コマ送りのアニメーション一つぶんを管理する。 
/// AnimationManagerに渡すと、状態に応じて服数のAnimationの再生を切り替えたり、制御することができる。
struct Animation {
private:
    Texture texture;
    double  spf;
    Size unit_sprite{64, 64};
    Size sprite_count;
    Interval<int> play_interval;
    bool loop = true;
    double second = 0;
public:
    /// @param texture_ 再生するスプライト
    /// @param spf_ second per frame (一フレームあたり秒数)
    /// @param unit_sprite_ 単位コマあたり大きさ
    /// @param sprite_count_ スプライトの個数。縦にm個, 横にn個単位コマがあったときにはSize{m, n}を渡す。
    Animation(
        const Texture& texture_,
        const double& spf_,
        const Size& unit_sprite_,
        const Size& sprite_count_
    ):
        texture(texture_),
        spf(spf_),
        unit_sprite(unit_sprite_),
        sprite_count(sprite_count_),
        play_interval(0, sprite_count.area())
    {
        assert((unit_sprite * sprite_count).x <= texture.size().x);
        assert((unit_sprite * sprite_count).y <= texture.size().y);
    }

    /// @brief 指定したアニメーションの再生が終了したか否かを通知する。
    bool arrive_at_end() const;
    
    /// @brief 今再生されているアニメーションがどの再生区間で再生するかを指定する。
    /// @param interval [m, n]で指定されていた時、mフレーム目から、nフレーム目 **の直前** までを再生する。最後が再生区間に含まれないことに注意。
    /// @return 設定対象のアニメーションを返す。これを用いると、メソッドチェーン形式で設定を記述できる。
    Animation& set_interval(const Interval<int>& interval);
    
    /// @brief アニメーションを再生した時に、それがループするか否かを指定する。 
    /// @param looping 
    /// @return 設定対象のアニメーションを返す。これを用いると、メソッドチェーン形式で設定を記述できる。
    Animation& set_loop(const bool looping);   
    
    /// @brief アニメーションを、delta_time秒経過した分だけ先に送る。set_loopでループ指定が行われていなかった場合にはそのまま再生を停止する。
    /// @param delta_time 
    void Update(float delta_time);

    /// @brief 再生するコマの個数を返す。
    int number_of_all_frame() const;

    /// @brief 今再生されているアニメーションがどの番号のコマを表示するかを返す。番号は0オリジンで管理されている。
    int frame_number() const;
    
    /// @brief 今再生されているアニメーションがどのコマを表示するかを返す。
    TextureRegion sprite() const;

    /// @brief 初め(つまり0コマ目)から再生する。
    void play_from_first();

    /// @brief sprite per secondsを調節する。
    void adjust_spf(const double spf);
};