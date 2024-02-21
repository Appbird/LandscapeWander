# pragma once
# include <Siv3D.hpp>

// quoted from https://siv3d.github.io/ja-jp/samples/ui/#10-%E3%82%BF%E3%82%A4%E3%83%AB%E5%9E%8B%E3%81%AE%E3%83%9C%E3%82%BF%E3%83%B3
// Siv3D UI のサンプル -- Accessed at 2024/02/20 23:13

class TileButton
{
public: 
    bool is_selected = false;

	struct Palette
	{
		ColorF tileColor1;

		ColorF tileColor2;

		ColorF borderColor1;

		ColorF borderColor2;
	};
    
	TileButton() = default;

	TileButton(const Icon& icon, int32 iconSize, const Font& font, double fontSize, const String& text, const RectF& rect, const Palette& palette)
		: TileButton{ Texture{ icon, iconSize }, iconSize, font, fontSize, text, rect, palette } {}

	// Texture からアイコンを作成
	TileButton(const TextureRegion& textureRegion, int32 iconSize, const Font& font, double fontSize, const String& text, const RectF& rect, const Palette& palette)
		: m_icon{ textureRegion }
		, m_iconSize{ iconSize }
		, m_font{ font }
		, m_fontSize{ fontSize }
		, m_text{ text }
		, m_rect{ rect }
		, m_palette{ palette } {}

	bool update()
	{
		const bool mouseOver = m_rect.mouseOver();

		bool pushed = false;

		if (mouseOver)
		{
			Cursor::RequestStyle(CursorStyle::Hand);
		}

		if (not m_pressed)
		{
			if (m_rect.leftClicked())
			{
				m_pressed = true;
			}
		}
		else
		{
			if (m_rect.leftReleased())
			{
				m_pressed = false;
				pushed = true;
			}
			else if (not m_rect.mouseOver())
			{
				m_pressed = false;
			}
		}

		m_transitionPressed.update(is_selected);

		return pushed;
	}

	void draw() const
	{
		const double t = m_transitionPressed.value();

		const Transformer2D transform{ Mat3x2::Scale((1 + t * 0.06), m_rect.center()) };

		// タイル
		{
			m_rect.draw(m_palette.tileColor1.lerp(m_palette.tileColor2, t));

			m_rect.stretched(Math::Lerp(-InnerBorderMargin, 0, t))
				.drawFrame(0.1, (1.0 + t * 2.0), m_palette.borderColor1.lerp(m_palette.borderColor2, t));
		}

		// アイコン
		{
			m_icon
				.drawAt(m_rect.getRelativePoint(0.5, 0.4), m_palette.tileColor2.lerp(m_palette.tileColor1, t));
		}

		// ラベル
		{
			m_font(m_text)
				.drawAt(m_fontSize, m_rect.getRelativePoint(0.5, 0.8), m_palette.tileColor2.lerp(m_palette.tileColor1, t));
		}
	}

private:

	static constexpr double InnerBorderMargin = 3.0;

	TextureRegion m_icon;

	int32 m_iconSize = 0;

	Font m_font;

	double m_fontSize = 0;

	String m_text;

	RectF m_rect;

	Transition m_transitionPressed{ 0.09s, 0.12s };

	Palette m_palette;

	bool m_pressed = false;

};