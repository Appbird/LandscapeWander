#pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.2

struct BubbleEffect : IEffect
{
	struct Bubble
	{
		Vec2 offset;
		double startTime;
		double scale;
		ColorF color;
	};

	Vec2 m_pos;

    Bubble main_bubble;
	Array<Bubble> m_bubbles;

	BubbleEffect(const Vec2& pos, const double momentum_)
		: m_pos{ pos }
	{
        assert(momentum_ > 0);
        main_bubble = Bubble{
            .offset = Vec2::Zero(),
            .startTime = 0,
            .scale = momentum_,
            .color = HSV{ 0, 0, 0.9},
        };
		for (int32 i = 0; i < 24 * momentum_; ++i)
		{
			Bubble bubble{
				.offset = RandomVec2(Circle{0.3}),
				.startTime = Random(-0.1, 0.1), // 登場の時間差
				.scale = Random(0.1, momentum_),
				.color = HSV{ 0, 0, 0.9}
			};
			m_bubbles << bubble;
		}
	}

	bool update(double t) override
	{
        const double t1 = (main_bubble.startTime + t);
        if (InRange(t1, 0.0, 1.0)) {
            const double e = EaseOutExpo(t1);
            Circle{ (m_pos), (e * 3.0 * main_bubble.scale) }
				.draw(ColorF{ main_bubble.color, 0.15 * (1-t*t) })
				.drawFrame((1.5 * (1.0 - e) * main_bubble.scale), main_bubble.color);
        }

		for (const auto& bubble : m_bubbles)
		{
			const double t2 = (bubble.startTime + t);

			if (not InRange(t2, 0.0, 1.0))
			{
				continue;
			}

			const double e = EaseOutExpo(t2);
            // 自由落下を加える
			Circle{ (m_pos + bubble.offset + (bubble.offset * 8 * t + Vec2{0, abs(bubble.offset.y)* 8}*t*t)), (0.15 * bubble.scale) }
				.draw(ColorF{ bubble.color, 0.15 * (1-t*t) })
				.drawFrame((1.5 * (1.0 - e) * bubble.scale), bubble.color);
		}

		return (t < 1.3);
	}
};
