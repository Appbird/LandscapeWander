#pragma once
#include <Siv3D.hpp>
#include "AnimationManager.hpp"
#include "CollisionEvent.hpp"

enum PlayerState{
    S_NULL,
    S_Starting,
    S_Running,
    S_Sliding,
    S_Waiting,
    S_Charged_in_Air,
    S_Prepare_Jump,
    S_Jump,
    S_Fall,
    S_Landing,
};
enum LookingDirection {
    LD_LEFT,
    LD_RIGHT
};

enum PlayerAnimationState {
    Slow_Running,
    Running,
    Sliding,
    Fast_Running,
    Waiting,
    Jumping_Up,
    Jumping_Down,
    Landing
};
struct Transform {
    Vec2 position;
    Vec2 velocity;
    Vec2 accelelation;
};

class Player { 
public:
    struct InputInfo {
        int direction;
        Vec2 jumping_direction;
        // ジャンプボタンが押されたかどうか
        bool jump_pressed;
    };
public:
    using AnimationManager = AnimationsManager<PlayerAnimationState>;
    
    Effect effect;

    Texture run;
    Texture jump;
    Audio run_se;
    Audio jump_se;
    Audio rocket_se;
    Audio land_se;
    Audio sliding_se;

    Transform transform_;
    SizeF character_size_{3.6, 3.6};

    // #FIXME Lineの寿命を考慮していないことに注意
    const Line* touched_ground = nullptr;
    double move_speed_ = 30;
    double jump_velocity_max = 24;
    double rundust_time = 0;
    double jump_effect_time = 0;
    
    // 滑りによるチャージ値
    // 上限値は1とする。
    double charged = 0;
    
    PlayerState p_state_ = S_Waiting;
    LookingDirection looking_direction_ = LD_RIGHT;

    AnimationManager animation_;

    Stopwatch starting_pose_stopwatch;
    const double starting_pose_period = 0.8;

    bool should_running = true;
    bool controllable_state = true;

    bool is_jumping() const;
    bool is_movable() const;
    bool is_runnable() const;
    bool is_on_ground() const;
    bool is_sliding() const;
    
    double running_momentum_percent() const;

    void stop_running();
private:
    // --- 関連関数群 (状態更新に係るもの)---

    /// @brief アニメーション群をロードする。
    /// 生成したAnimationManagerオブジェクト
    AnimationManager prepare_animation();

    /// @brief 常に実行されるルーチン
    /// @param input 
    void on_always(const InputInfo& input);
    
    /// @brief 空中にいるときに実行されるルーチン
    void on_in_air(const InputInfo& input);
    /// @brief 地面にいるときに実行されるルーチン
    void on_ground(const InputInfo& input);

    /// @brief スライディングしている時に実行されるルーチン
    void on_sliding(const InputInfo& input);
    
    /// @brief ジャンプが準備中であるときに実行されるルーチン
    void on_jump_start(const InputInfo& input);
    /// @brief 着地するときに実行されるルーチン
    void on_landing(const InputInfo& input);
    
    /// @brief 走ることが可能な場合に実行されるルーチン
    void on_runnable(const InputInfo& input);
    /// @brief 動くことが可能な場合に実行されるルーチン
    void on_movable(const InputInfo& input);

    void on_start_off_ground(const InputInfo& input);
    

    void on_brake(const double deacc_coef, const double charged_coef);
    void occur_rundust_effect(const double rundust_interval_time);

public:
    Player(Vec2 position):
        run{ U"../assets/sprites/stickfigure_walk.png" },
        jump{ U"../assets/sprites/stickfigure_jump.png" },
        run_se{ U"../assets/se/running.wav" },
        jump_se{ U"../assets/se/jump.wav" },
        land_se{ U"../assets/se/land.wav" },
        sliding_se{ U"../assets/se/sliding.mp3" },
        rocket_se{ U"../assets/se/rocket.mp3" },
        animation_(prepare_animation())
    {
        run_se.setLoop(true);
        transform_.position = position;
        animation_.change_animation(Waiting);
    }
    
    void update(Effect& effect);
    void resolve_collision(const Array<CollisionEvent>& collisionEvents, const Array<Line>& lines);
    void draw() const;
    
    Line collision_line() const;
    RectF collision_box() const;
    Line landing_raycast() const;
    Vec2 foot_point() const;
    
};

/*
    アプローチを決めたい
    (1) 状態変数は引数越しで読ませる？
        (+) テスト性が容易
        (+) ECSの形に持っていきやすい
        (+) どの変数に依存して値が決定されているかが明示的になる
        (-) 引数が多くなる
        (-) 変数が6個を超えた場合にキャッシュを生かしにくくなる
            ---> それ専用のバンドルを作ればいい
        (-) インタフェースが安定しない(?)
    (2) 状態変数はconst参照で読ませる？
        (+) 状態の全てを読むことができる。
        (+) 引数がそれほど大きくならない。
        (+) 外部インタフェースを用意しやすい。(?)
        (-) テストの際に、プレイヤーの状態を全て打ち込む必要がある
        (-) どの関数がどの変数に依存するのかがわからない
        (-) ECSに転用できない
        (-) コンテキストが生まれる

    ... でも、テストはしない！から慣れてる方を取る！
*/