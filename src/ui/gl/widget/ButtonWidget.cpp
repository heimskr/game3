#include "game/ClientGame.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "graphics/TextRenderer.h"
#include "threading/ThreadContext.h"
#include "ui/gl/widget/ButtonWidget.h"
#include "ui/gl/UIContext.h"

#include <cassert>

namespace {
	constexpr Game3::Color DEFAULT_BORDER_COLOR{"#5d515a"};
	constexpr Game3::Color DEFAULT_TEXT_COLOR{"#000000"};
}

namespace Game3 {
	ButtonWidget::ButtonWidget(float scale, float fixed_height, Color top_border_color, Color bottom_border_color, Color text_color, TexturePtr texture):
		Widget(scale),
		fixedHeight(fixed_height),
		texture(std::move(texture)) {
			setColors(top_border_color, bottom_border_color, text_color);
		}

	ButtonWidget::ButtonWidget(float scale, float fixed_height, Color border_color, Color text_color, TexturePtr texture):
		ButtonWidget(scale, fixed_height, border_color, border_color.darken(), text_color, std::move(texture)) {}

	ButtonWidget::ButtonWidget(float scale, float fixed_height, TexturePtr texture):
		ButtonWidget(scale, fixed_height, DEFAULT_BORDER_COLOR, DEFAULT_TEXT_COLOR, std::move(texture)) {}

	void ButtonWidget::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		if (fixedHeight > 0)
			height = fixedHeight;

		adjustWidth(renderers, width, height);
		Widget::render(ui, renderers, x, y, width, height);

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

		const float value = pressed? 1.5 : 1;

		assert(texture);
		renderers.singleSprite.drawOnScreen(texture, RenderOptions{
			.x = x + scale,
			.y = adjusted_y + scale,
			.sizeX = width - 2 * scale,
			.sizeY = height - 4 * scale,
			.scaleX = scale,
			.scaleY = scale,
			.color{value, value, value, 1},
			.invertY = false,
			.wrapMode = GL_REPEAT,
		});

		{
			const Rectangle rectangle(x + scale, adjusted_y + scale, width - 2 * scale, height - 4 * scale);
			auto saver = ui.scissorStack.pushRelative(rectangle, renderers);
			renderLabel(ui, renderers, rectangle.width, rectangle.height);
		}

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

	bool ButtonWidget::click(UIContext &, int, int, int) {
		return false;
	}

	bool ButtonWidget::dragStart(UIContext &ui, int, int) {
		pressed = true;
		ui.getGame()->playSound("base:sound/click", threadContext.getPitch(1.25));
		ui.setPressedWidget(weak_from_this());
		return true;
	}

	bool ButtonWidget::dragEnd(UIContext &ui, int x, int y) {
		if (pressed) {
			pressed = false;
			ui.unpress();
			if (lastRectangle.contains(x, y)) {
				if (onClick)
					onClick(*this, ui, 1, x - lastRectangle.x, y - lastRectangle.y);
			}
		}

		return true;
	}

	float ButtonWidget::calculateHeight(const RendererContext &, float, float available_height) {
		return fixedHeight > 0? fixedHeight : available_height;
	}

	const Glib::ustring & ButtonWidget::getText() const {
		return text;
	}

	float ButtonWidget::getFixedHeight() const {
		return fixedHeight;
	}

	void ButtonWidget::setText(Glib::ustring new_text) {
		text = std::move(new_text);
	}

	void ButtonWidget::setFixedHeight(float new_fixed_height) {
		fixedHeight = new_fixed_height;
	}

	void ButtonWidget::renderLabel(UIContext &, const RendererContext &renderers, float, float height) {
		if (text.empty())
			return;

		const float text_scale = getTextScale(renderers, height - 2 * scale);
		renderers.text.drawOnScreen(text, TextRenderOptions{
			.x = 2 * scale,
			.y = height - scale,
			.scaleX = text_scale,
			.scaleY = text_scale,
			.color = pressed? textColorPressed : textColor,
			.alignTop = false,
		});
	}

	void ButtonWidget::adjustWidth(const RendererContext &renderers, float &width, float height) const {
		if (!text.empty()) {
			const float text_scale = getTextScale(renderers, height - 6 * scale);
			width = scale * 6 + renderers.text.textWidth(text, text_scale);
		}
	}

	float ButtonWidget::getTextScale(const RendererContext &renderers, float height) const {
		return height / renderers.text.getIHeight(scale / 8);
	}

	void ButtonWidget::setColors(Color top, Color bottom, Color text_color) {
		topBorderColor = top;
		bottomBorderColor = bottom;
		textColor = text_color;
		topBorderColorPressed = top.multiplyValue(3).desaturate();
		bottomBorderColorPressed = bottom.multiplyValue(3).desaturate();
		textColorPressed = text_color;
	}

	TexturePtr ButtonWidget::getDefaultTexture() {
		return cacheTexture("resources/gui/stone.png");
	}
}
