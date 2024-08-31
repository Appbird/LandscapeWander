#include "Player.hpp"
#include "../Utility/bubble.hpp"
#include "../Utility/RunDustEffect.hpp"

// COMPLETE #TODO 降りている最中にジャンプできなくなる問題
namespace LandscapeExtenders {

bool CONTROLL_METHOD = true;

static constexpr double gravity = 9.8;

static double sq(const double x) {
    return x * x;
}

static double sign(const double x) {
    if (x > 0) { return 1; }
    if (x < 0) { return -1; }
    return 0;
}


bool Player::is_jumping() const {
    return p_state_ == S_Jump or p_state_ == S_Fall;
}
bool Player::is_movable() const {
    return is_runnable() or p_state_ == S_Jump or p_state_ == S_Fall;
}
bool Player::is_runnable() const {
    return p_state_ == S_Running or p_state_ == S_Waiting;
}
bool Player::is_on_ground() const {
    return touched_ground != nullptr;
}

AnimationsManager<PlayerAnimationState> Player::prepare_animation() {
    return {{
        { 
            Running,
            Animation{run,   0.08, {64, 64}, {4, 2} }
        },
        { 
            Waiting,
            Animation{jump,  0.05, {64, 64}, {4, 2} }
            .set_loop(false)
            .set_interval({0, 1})
        },
        { 
            Jumping_Up,  
            Animation{jump,  0.05, {64, 64}, {4, 2} }
            .set_loop(false)
            .set_interval({0, 4})
        },
        { 
            Jumping_Down,  
            Animation{jump,  0.1, {64, 64}, {4, 2} }
            .set_loop(false)
            .set_interval({4, 5})
        },
        { 
            Landing,  
            Animation{jump,  0.1, {64, 64}, {4, 2} }
            .set_loop(false)
            .set_interval({5, 8})
        },
    }};
}



static int interpret_movement_direction(bool is_movable) {
    if (not is_movable) { return 0; }
    if (CONTROLL_METHOD) {
        if (const auto r = JoyCon(0)) {
            if (r.povD8() == 2) {
                return 1;
            } else if (r.povD8() == 6) {
                return -1;
            }
        }
    } else {
        if (KeyLeft.pressed()) {
            return -1;
        } else if (KeyRight.pressed()) {
            return 1;
        }
    }
    return 0;
}

static bool interpret_jumping(bool is_movable) {
    if (CONTROLL_METHOD) {
        if (const auto r = JoyCon(0)) {
            return (r.button1.pressed());
        }
        return false;
    } else {
        return KeySpace.pressed();
    }
}

void Player::update(Effect& effect) {
    const InputInfo input {
        interpret_movement_direction(is_movable()),
        interpret_jumping(is_movable())
    };
    if (is_movable()) {
        if (input.direction == -1) {
            looking_direction_ = LD_LEFT;
        } else if (input.direction == 1) {
            looking_direction_ = LD_RIGHT;
        }
    }

    on_always(input);
    // #FIXED ジャンプができない
    if (is_movable())   { on_movable(input);  }
    if (is_runnable())  { on_runnable(input, effect); }
    if (is_on_ground()) { on_ground(input);   } else { on_in_air(input); }

    switch (p_state_) {
        case S_Prepare_Jump:
            on_jump_start(input, effect);
            break;
        case S_Landing:
            on_landing(input);
            break;
        default:
            break;
    }
    transform_.position += transform_.velocity * Scene::DeltaTime();
    transform_.velocity += transform_.accelelation * Scene::DeltaTime();
    animation_.update(Scene::DeltaTime());
}


void Player::resolve_collision(const CollisionTicket& collision_event) {
    const double y_begin = collision_line().begin.y;
    const double y_end = collision_line().end.y;
    // lineが動く場合、lineの動きも考慮する必要があるが、今回は考えない。
    // 下方向につっこんでいた場合
    if (
        collision_event.collided->intersects(collision_line())
        and collision_event.collided->intersects(landing_raycast())
        and transform_.velocity.y > 0
    ) {
        if (transform_.velocity.y > gravity/2) { land_se.play(); }
        transform_.velocity.y = 0;
        transform_.position.y += collision_event.collided_point.y - std::max(y_begin, y_end);
        touched_ground = collision_event.collided;
    }
}

void Player::draw() const{
    animation_.sprite()
        .resized(character_size_)
        .mirrored(looking_direction_ == LD_LEFT)
        .drawAt(transform_.position);
}



void Player::on_always(const InputInfo& input) {
    transform_.accelelation = {0, 0};
    transform_.accelelation += - 0.3 * transform_.velocity;
    transform_.accelelation += Vec2{0, gravity};
}

void Player::on_jump_start(const InputInfo& input, Effect& effect) {
    if (animation_.current_animation().arrive_at_end()) {
        jump_se.play();
        // 走る速度に応じてジャンプの勢いを変えるようにする。
        const double momentum_coef = (running_momentum_percent() * 0.4 + 0.6);
        transform_.velocity.y = -jump_velocity_max * momentum_coef;
        p_state_ = S_Jump;
        on_start_off_ground(input);
        effect.add<BubbleEffect>(this->foot_point(), momentum_coef);
    }
}

void Player::on_start_off_ground(const InputInfo& input) {
    run_se.stop();
    touched_ground = nullptr;
}

void Player::on_in_air(const InputInfo& input) {
    if (transform_.velocity.y > gravity / 10) {
        animation_.change_animation_ignorable(Jumping_Down);
    }
    if (transform_.velocity.y > gravity / 10) {
        p_state_ = S_Fall;
    }
}

double Player::running_momentum_percent() const {
    return abs(Clamp(abs(transform_.velocity.x) / move_speed_, -1.0, 1.0));
}

void Player::on_ground(const InputInfo& input) {
    //着地した時
    if (is_jumping()) {
        p_state_ = S_Landing;
        animation_.change_animation(Landing);
    }
    if (input.jump_pressed and p_state_ != S_Prepare_Jump) {
        p_state_ = S_Prepare_Jump;
        animation_.change_animation(Jumping_Up);
        animation_.change_spf((1 - running_momentum_percent() * 0.6) * 0.08);
    }
    
    const auto touched_point = (touched_ground != nullptr) ? touched_ground->intersectsAt(landing_raycast()) : none;
    if (touched_point) {
        transform_.velocity.y = 0;
        transform_.position.y -= foot_point().y - touched_point->y;
    } else {
        on_start_off_ground(input);
    }
    
}

void Player::on_landing(const InputInfo& input) {
    if (animation_.current_animation().arrive_at_end()) {
        p_state_ = S_Waiting;
        animation_.change_animation(Waiting);
    }
}


// 実写とCGアニメ

void Player::on_runnable(const InputInfo& input, Effect& effect) {
    if (p_state_ != S_Waiting and input.direction == 0) {
        run_se.stop();
        p_state_ = S_Waiting;
        animation_.change_animation(Waiting);
    } else if (p_state_ != S_Running and input.direction != 0) {
        run_se.play();
        p_state_ = S_Running;
        animation_.change_animation(Running);
    } else {}
    // 走る速度によってアニメスピードを変える。
    if (p_state_ == S_Running and abs(transform_.velocity.x) > 1e-6) { 
        run_se.setSpeed((running_momentum_percent() * 0.6 + 0.4));
        animation_.change_spf((1 - running_momentum_percent() * 0.6) * 0.08);
        
        rundust_time += Scene::DeltaTime();
        if (rundust_time > rundust_interval_time) {
            rundust_time -= rundust_interval_time;
            effect.add<RunDustEffect>(
                transform_.position + Vec2{0, character_size_.y / 2},
                sign(transform_.velocity.x)
            );
        }
    }
}

void Player::on_movable(const InputInfo& input) {
    const double move_acc_coef = (is_on_ground()) ? 2 : 1;
    // 以下の微分方程式に基づいてプレイヤーの速度を更新する。
    // dσ = (1-σ)σdx ---> σ = 1/(1+e^{-x}) (ロジスティックシグモイド関数)
    if (touched_ground) {
        move_speed_ = 13 * 0.75 * pow(2, gradarg_Line(*touched_ground, input.direction));
    }
    // 加速する時
    if (sign(transform_.velocity.x) == input.direction or sign(transform_.velocity.x) == 0) {
        double& v = transform_.velocity.x;
        v += input.direction * move_acc_coef / (2 * move_speed_) * (sq(move_speed_) - sq(v)) * Scene::DeltaTime();
    }
    // 減速する時
    if (sign(transform_.velocity.x) != input.direction and abs(transform_.velocity.x) > 1e-6){
        const double stop_time = (input.direction == 0) ? 2 : 1 ;
        const double current_direction = sign(transform_.velocity.x);
        transform_.velocity.x += -current_direction * move_speed_ / stop_time * Scene::DeltaTime();
        if (sign(transform_.velocity.x) == -current_direction) { transform_.velocity.x = 0; }
    }
    if (abs(transform_.velocity.x) <= 1e-6) { transform_.velocity.x = 0; }
}


Line Player::collision_line() const {
    Vec2 half_line{ 0, character_size_.y / 2 };
    return Line{transform_.position + half_line, transform_.position - half_line};
}

Line Player::landing_raycast() const {
    Vec2 half_size{ 0, character_size_.y / 2 *  1 / 2 };
    Vec2 delta_size{ 0, character_size_.y / 3 };
    const Vec2 begin_point = transform_.position + half_size;

    return Line{begin_point, begin_point + delta_size};
}
Vec2 Player::foot_point() const {
    Vec2 half_size{ 0, character_size_.y / 2};
    return transform_.position + half_size;
}
Vec2 Player::head_point() const {
    Vec2 half_size{ 0, character_size_.y / 2};
    return transform_.position - half_size;
}
}