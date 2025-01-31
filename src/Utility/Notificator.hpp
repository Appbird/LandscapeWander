# include <Siv3D.hpp>

/// @brief 通知を管理するアドオン
class NotificationAddon : public IAddon
{
public:

	/// @brief 通知の種類
	enum class Type
	{
		/// @brief 通常
		Normal,

		/// @brief 情報
		Information,

		/// @brief 疑問
		Question,

		/// @brief 成功
		Success,

		/// @brief 警告
		Warning,

		/// @brief 失敗
		Failure,
	};

	/// @brief 通知のスタイル
	struct Style
	{
		/// @brief 通知の幅
		double width = 600.0;

		/// @brief 通知の背景色
		ColorF backgroundColor{ 0.0, 0.8 };

		/// @brief 通知の枠線色
		ColorF frameColor{ 0.75 };

		/// @brief 通知の文字色
		ColorF textColor{ 1.0 };

		/// @brief 情報アイコンの色
		ColorF informationColor{ 0.0, 0.72, 0.83 };

		/// @brief 疑問アイコンの色
		ColorF questionColor{ 0.39, 0.87, 0.09 };

		/// @brief 成功アイコンの色
		ColorF successColor{ 0.0, 0.78, 0.33 };

		/// @brief 警告アイコンの色
		ColorF warningColor{ 1.0, 0.57, 0.0 };

		/// @brief 失敗アイコンの色
		ColorF failureColor{ 1.00, 0.32, 0.32 };
	};

	/// @brief 通知を表示します。
	/// @param message メッセージ
	/// @param type 通知の種類
	static void Show(const StringView message, const Type type = NotificationAddon::Type::Normal)
	{
		if (auto p = Addon::GetAddon<NotificationAddon>(U"NotificationAddon"))
		{
			p->show(message, type);
		}
	}

	/// @brief 通知の表示時間を設定します。
	/// @param lifeTime 表示時間（秒）
	static void SetLifeTime(const double lifeTime)
	{
		if (auto p = Addon::GetAddon<NotificationAddon>(U"NotificationAddon"))
		{
			p->m_lifeTime = lifeTime;
		}
	}

	/// @brief 通知のスタイルを設定します。
	/// @param style スタイル
	static void SetStyle(const Style& style)
	{
		if (auto p = Addon::GetAddon<NotificationAddon>(U"NotificationAddon"))
		{
			p->m_style = style;
		}
	}

private:

	static constexpr StringView Icons = U" \U000F02FC\U000F02D7\U000F0E1E\U000F0029\U000F1398";

	struct Notification
	{
		String message;

		double time = 0.0;

		double currentIndex = 0.0;

		double velocity = 0.0;

		Type type = Type::Normal;
	};

	Style m_style;

	Array<Notification> m_notifications;

	double m_lifeTime = 10.0;

	bool update() override
	{
		const double deltaTime = Scene::DeltaTime();

		for (auto& notification : m_notifications)
		{
			notification.time += deltaTime;
		}

		m_notifications.remove_if([lifeTime = m_lifeTime](const Notification& notification) { return (lifeTime < notification.time); });

		for (size_t i = 0; i < m_notifications.size(); ++i)
		{
			auto& notification = m_notifications[i];
			notification.currentIndex = Math::SmoothDamp(notification.currentIndex,
				static_cast<double>(i), notification.velocity, 0.15, 9999.0, deltaTime);
		}

		return true;
	}

	void draw() const override
	{
		const Font& font = SimpleGUI::GetFont();

		for (const auto& notification : m_notifications)
		{
			double xScale = 1.0;
			double alpha = 1.0;

			if (notification.time < 0.2)
			{
				xScale = alpha = (notification.time / 0.2);
			}
			else if ((m_lifeTime - 0.4) < notification.time)
			{
				alpha = ((m_lifeTime - notification.time) / 0.4);
			}

			alpha = EaseOutExpo(alpha);
			xScale = EaseOutExpo(xScale);

			ColorF backgroundColor = m_style.backgroundColor;
			backgroundColor.a *= alpha;

			ColorF frameColor = m_style.frameColor;
			frameColor.a *= alpha;

			ColorF textColor = m_style.textColor;
			textColor.a *= alpha;

			const RectF rect{ 10, (10 + notification.currentIndex * 32), (m_style.width * xScale), 31 };
			rect.rounded(3).draw(backgroundColor).drawFrame(1, 0, frameColor);

			if (notification.type != Type::Normal)
			{
				ColorF color = notification.type == Type::Information ? m_style.informationColor
					: notification.type == Type::Question ? m_style.questionColor
					: notification.type == Type::Success ? m_style.successColor
					: notification.type == Type::Warning ? m_style.warningColor
					: m_style.failureColor;
				color.a *= alpha;

				font(Icons[FromEnum(notification.type)]).draw(18, Arg::leftCenter = rect.leftCenter().movedBy(8, -1), color);
			}

			font(notification.message).draw(18, Arg::leftCenter = rect.leftCenter().movedBy(32, -1), textColor);
		}
	}

	void show(const StringView message, const Type type)
	{
		const double currentIndex = (m_notifications.empty() ? 0.0 : m_notifications.back().currentIndex + 1.0);
		const double velocity = (m_notifications.empty() ? 0.0 : m_notifications.back().velocity);

		m_notifications << Notification{
			.message = String{ message },
			.time = 0.0,
			.currentIndex = currentIndex,
			.velocity = velocity,
			.type = type };
	}
};