#pragma once
#include "Animation.hpp"
#include <Siv3D.hpp>

/// @brief 複数のアニメーションの再生. 切り替えを制御するオブジェクト
/// @tparam AnimationState 
template <typename AnimationState>
class AnimationsManager{
private:
    AnimationState animation_state = static_cast<AnimationState>(0);
    std::map<AnimationState, Animation> animations;
    
public:
    /// @param animations アニメーションの列挙子とAnimationの組を列挙
    AnimationsManager(
        std::map<AnimationState, Animation>&& animations_
    ):
        animations(animations_)
    {}
    
    /// @param 現在再生されているアニメーションを返す。
    const Animation& current_animation() const{
        return animations.at(animation_state);
    }

    /// @param nextに指定された状態のアニメーションに切り替える。
    void change_animation(AnimationState next) {
        assert(animations.contains(next));
        animation_state = next;
        animations.at(animation_state).play_from_first();
    }
    
    /// @param nextに指定された状態のアニメーションに切り替える。ただし、同じ状態が指定された場合には初めから再生しない。
    void change_animation_ignorable(AnimationState next) {
        assert(animations.contains(next));
        if (animation_state == next) { return; }
        animation_state = next;
        animations.at(animation_state).play_from_first();
    }

    /// @brief 現在再生されているアニメーションのスプライトを得る。
    TextureRegion sprite() const {
        return current_animation().sprite();
    }

    /// @brief アニメーションをdelta_seconds秒ぶんだけ経過したものとしてコマ送りする。
    void update(const double delta_seconds) {
        return animations.at(animation_state).Update(delta_seconds);
    }

    void change_spf(const double spf) {
        animations.at(animation_state).adjust_spf(spf);
    }
    
    
};