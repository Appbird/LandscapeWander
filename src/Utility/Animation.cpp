#include "Animation.hpp"

void Animation::Update(float delta_time) {
    second += delta_time;
}
int Animation::number_of_all_frame() const {
    return (sprite_count.x * sprite_count.y); 
}
int Animation::frame_number() const {
    const int flipped_frame = static_cast<int>(second / spf);
    const int delta = 
        (not this->loop)
            ? Clamp(flipped_frame, 0, this->play_interval.length() - 1)
            : (flipped_frame % this->play_interval.length());
    return this->play_interval.begin + delta;
}
TextureRegion Animation::sprite() const {
    assert(InRange(this->frame_number(), 0, number_of_all_frame() - 1));
    const int width     = this->frame_number() % sprite_count.x;
    const int height    = this->frame_number() / sprite_count.x; 
    assert(InRange(width, 0, sprite_count.x));
    assert(InRange(height, 0, sprite_count.y));
    return texture(unit_sprite * Point{width, height}, unit_sprite);
}

Animation& Animation::set_interval(const Interval<int>& interval) {
    this->play_interval = interval;
    return *this;
}
Animation& Animation::set_loop(const bool looping) {
    this->loop = looping;
    return *this;
}

void Animation::play_from_first(){
    this->second = 0;
}

bool Animation::arrive_at_end() const {
    return this->frame_number() == this->play_interval.last();
}

void Animation::adjust_spf(const double spf_) {
    assert(spf_ > 0);
    spf = spf_;
}