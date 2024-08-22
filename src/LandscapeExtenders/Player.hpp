#pragma once
#include <Siv3D.hpp>
#include "../Utility/AnimationManager.hpp"
#include "World.hpp"

namespace LandscapeExtenders {

enum PlayerState{
    S_NULL,
    S_Running,
    S_Waiting,
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
        // ジャンプボタンが押されたかどうか
        bool jump_pressed;
    };
public:
    using AnimationManager = AnimationsManager<PlayerAnimationState>;
    
    Texture run;
    Texture jump;
    Audio run_se;
    Audio jump_se;
    Audio land_se;

    Transform transform_;
    SizeF character_size_{1.8, 1.8};

    // #FIXME Lineの寿命を考慮していないことに注意
    std::shared_ptr<Line> touched_ground = nullptr;
    double move_speed_ = 13;
    double jump_velocity_max = 15;
    double rundust_time = 0;
    const double rundust_interval_time = 0.2;
    


    PlayerState p_state_ = S_Waiting;
    LookingDirection looking_direction_ = LD_LEFT;

    AnimationManager animation_;

    bool is_jumping() const;
    bool is_movable() const;
    bool is_runnable() const;
    bool is_on_ground() const;
    
    double running_momentum_percent() const;

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
    
    /// @brief ジャンプが準備中であるときに実行されるルーチン
    void on_jump_start(const InputInfo& input, Effect& effect);
    /// @brief 着地するときに実行されるルーチン
    void on_landing(const InputInfo& input);
    
    /// @brief 走ることが可能な場合に実行されるルーチン
    void on_runnable(const InputInfo& input, Effect& effect);
    /// @brief 動くことが可能な場合に実行されるルーチン
    void on_movable(const InputInfo& input);

    void on_start_off_ground(const InputInfo& input);
    
public:
    Player():
        run{ TextureAsset(U"player/run") },
        jump{ TextureAsset(U"player/jump") },
        run_se{ AudioAsset(U"se/run") },
        jump_se{ AudioAsset(U"se/jump") },
        land_se{ AudioAsset(U"se/land") },
        animation_(prepare_animation())
    {
        run_se.setLoop(true);
    }
    void Init(Vec2 position) {
        transform_.position = position;
        animation_.change_animation(Waiting);
    }
    void update(Effect& effect);
    void resolve_collision(const CollisionTicket& collisionEvents);
    void draw() const;
    
    Line collision_line() const;
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
}