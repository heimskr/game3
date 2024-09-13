#include "game/ClientGame.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "graphics/TextRenderer.h"
#include "threading/ThreadContext.h"
#include "ui/gl/widget/Button.h"
#include "ui/gl/UIContext.h"

#include <cassert>

namespace {
	constexpr Game3::Color DEFAULT_BORDER_COLOR{"#5d515a"};
	constexpr Game3::Color DEFAULT_TEXT_COLOR{"#000000"};
}

namespace Game3 {
	Button::Button(UIContext &ui, float scale, Color top_border_color, Color bottom_border_color, Color text_color, TexturePtr texture):
		Widget(ui, scale),
		texture(std::move(texture)) {
			setColors(top_border_color, bottom_border_color, text_color);
		}

	Button::Button(UIContext &ui, float scale, Color border_color, Color text_color, TexturePtr texture):
		Button(ui, scale, border_color, border_color.darken(), text_color, std::move(texture)) {}

	Button::Button(UIContext &ui, float scale, TexturePtr texture):
		Button(ui, scale, DEFAULT_BORDER_COLOR, DEFAULT_TEXT_COLOR, std::move(texture)) {}

	void Button::render(const RendererContext &renderers, float x, float y, float width, float height) {
		const float original_width = width;
		const float original_height = height;

		fixSizes(width, height);
		float dummy{};
		if (fixedHeight <= 0 && height < 0) {
			measure(renderers, Orientation::Vertical, width, height, dummy, height);
		}

		measure(renderers, Orientation::Horizontal, width, height, dummy, width);

		adjustCoordinate(Orientation::Horizontal, x, original_width, width);
		adjustCoordinate(Orientation::Vertical, y, original_height, height);

		Widget::render(renderers, x, y, width, height);

		RectangleRenderer &rectangler = renderers.rectangle;
		const Color &top_color = pressed? topBorderColorPressed : topBorderColor;
		const Color &bottom_color = pressed? bottomBorderColorPressed : bottomBorderColor;
		const float adjusted_y = pressed? y + scale : y;

		// Top
		rectangler(top_color, x + 2 * scale, adjusted_y, width - 4 * scale, scale);

		// Bottom
		rectangler(top_color, x + 2 * scale, adjusted_y + height - 3 * scale, width - 4 * scale, scale);

		// Left
		rectangler(top_color, x, adjusted_y + 2 * scale, scale, height - 6 * scale);

		// Right
		rectangler(top_color, x + width - scale, adjusted_y + 2 * scale, scale, height - 6 * scale);

		assert(texture);
		renderers.singleSprite.drawOnScreen(texture, RenderOptions{
			.x = x + scale,
			.y = adjusted_y + scale,
			.sizeX = width - 2 * scale,
			.sizeY = height - 4 * scale,
			.scaleX = scale,
			.scaleY = scale,
			.color = textureMultiplierPressed,
			.invertY = false,
			.wrapMode = GL_REPEAT,
		});

		renderLabel(renderers, Rectangle(x + scale, adjusted_y + scale, width - 2 * scale, height - 4 * scale));

		// Top left
		rectangler(top_color, x + scale, adjusted_y + scale, scale, scale);

		// Top right
		rectangler(top_color, x + width - 2 * scale, adjusted_y + scale, scale, scale);

		// Bottom left
		rectangler(top_color, x + scale, adjusted_y + height - 4 * scale, scale, scale);

		// Bottom right
		rectangler(top_color, x + width - 2 * scale, adjusted_y + height - 4 * scale, scale, scale);

		const float bottom_height = pressed? scale : 2 * scale;

		// Left
		rectangler(bottom_color, x, adjusted_y + height - 4 * scale, scale, bottom_height);
		rectangler(bottom_color, x + scale, adjusted_y + height - 3 * scale, scale, bottom_height);

		// Right
		rectangler(bottom_color, x + width - scale, adjusted_y + height - 4 * scale, scale, bottom_height);
		rectangler(bottom_color, x + width - 2 * scale, adjusted_y + height - 3 * scale, scale, bottom_height);

		// Bottom
		rectangler(bottom_color, x + 2 * scale, adjusted_y + height - 2 * scale, width - 4 * scale, bottom_height);
	}

	bool Button::click(int, int, int) {
		return false;
	}

	bool Button::dragStart(int, int) {
		pressed = true;
		ui.getGame()->playSound("base:sound/click", threadContext.getPitch(1.25));
		ui.setPressedWidget(shared_from_this());
		return true;
	}

	bool Button::dragEnd(int x, int y) {
		if (pressed) {
			pressed = false;
			ui.unpress();
			if (lastRectangle.contains(x, y)) {
				if (onClick)
					onClick(*this, 1, x - lastRectangle.x, y - lastRectangle.y);
			}
		}

		return true;
	}

	SizeRequestMode Button::getRequestMode() const {
		return SizeRequestMode::WidthForHeight;
	}

	void Button::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		if (orientation == Orientation::Horizontal) {
			if (horizontalExpand && 0 <= for_width) {
				minimum = 0;
				natural = for_width;
				return;
			}

			if (for_height < 0)
				for_height = std::max(fixedHeight, getMinimumPreferredHeight());

			minimum = natural = getWidth(renderers, for_height);
		} else {
			minimum = getMinimumPreferredHeight();
			natural = std::max(minimum, fixedHeight);
		}
	}

	const UString & Button::getText() const {
		return text;
	}

	void Button::setText(UString new_text) {
		text = std::move(new_text);
	}

	void Button::renderLabel(const RendererContext &renderers, const Rectangle &rectangle) {
		if (text.empty())
			return;

		const float text_scale = getTextScale(renderers, rectangle.height - 2 * scale);
		renderers.text.drawOnScreen(text, TextRenderOptions{
			.x = rectangle.x + 2 * scale,
			.y = rectangle.y + rectangle.height - scale,
			.scaleX = text_scale,
			.scaleY = text_scale,
			.color = pressed? textColorPressed : textColor,
			.alignTop = false,
		});
	}

	float Button::getWidth(const RendererContext &renderers, float height) const {
		if (!text.empty()) {
			const float text_scale = getTextScale(renderers, height - 6 * scale);
			return scale * 6 + renderers.text.textWidth(text, text_scale);
		}

		return -1;
	}

	float Button::getTextScale(const RendererContext &renderers, float height) const {
		return height / renderers.text.getIHeight(scale / 8);
	}

	void Button::setColors(Color top, Color bottom, Color text_color) {
		topBorderColor = top;
		bottomBorderColor = bottom;
		textColor = text_color;
		textColorPressed = text_color;
		topBorderColorPressed = top;
		bottomBorderColorPressed = bottom;
		// topBorderColorPressed = top.multiplyValue(3).desaturate();
		// bottomBorderColorPressed = bottom.multiplyValue(3).desaturate();
		// textureMultiplierPressed = {1.5, 1.5, 1.5, 1};
	}

	TexturePtr Button::getDefaultTexture() {
		return cacheTexture("resources/gui/stone.png");
	}

	float Button::getMinimumPreferredHeight() const {
		return scale * 12;
	}
}
