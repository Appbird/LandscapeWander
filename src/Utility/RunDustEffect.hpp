#pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.2

struct RunDustEffect : IEffect
{
	struct Bubble
	{
		Vec2 init_velocity;
		double startTime;
        double radius;
		ColorF color;
	};

	Vec2 m_pos;
	Array<Bubble> m_bubbles;

	RunDustEffect(const Vec2& pos, const int direction)
		: m_pos{ pos }
	{
		for (int32 i = 0; i < 3; ++i)
		{
			Bubble bubble{
				.init_velocity = RandomVec2(RectF{Vec2{-direction * 3.0, -3.0}, Vec2{3.0, 4.0}}),
				.startTime = Random(-0.1, 0.1), // 登場の時間差
                .radius = Random(0.1, 0.5),
				.color = HSV{ 0, 0, Random(0.6, 0.8) }
			};
			m_bubbles << bubble;
		}
	}

	bool update(double t) override
	{
		for (const auto& bubble : m_bubbles)
		{
			const double t2 = (bubble.startTime + t);

			if (not InRange(t2, 0.0, 1.0)) { continue; }

			const double e = EaseOutExpo(t2);
            // 自由落下を加える
			Circle{ (m_pos + (bubble.init_velocity * t - Vec2{0, 1.0 *2}*t*t)), bubble.radius * (1-t) }
				.draw(ColorF{ bubble.color, 0.60 * t })
				.drawFrame((bubble.radius * (1.0 - e)), bubble.color);
		}

		return (t < 0.8);
	}
};