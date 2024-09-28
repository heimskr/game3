#include "graphics/RendererContext.h"
#include "graphics/TextRenderer.h"
#include "graphics/Texture.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/Types.h"
#include "ui/gl/UIContext.h"
#include "ui/Window.h"

namespace Game3 {
	Label::Label(UIContext &ui, float scale, UString text, Color text_color):
		Widget(ui, scale),
		textColor(text_color) {
			setText(std::move(text));
		}

	void Label::render(const RendererContext &renderers, float x, float y, float width, float height) {
		if (lastRectangle.width != static_cast<int>(width)) {
			wrapped.reset();
		}

		const float padding = getPadding();

		tryWrap(renderers.text, width);
		const UString &string = wrapped? wrapped.value() : text;

		if (0 <= lastUnwrappedTextWidth)
			adjustCoordinate(Orientation::Horizontal, x, width, std::min(width, lastUnwrappedTextWidth));

		Widget::render(renderers, x, y, width, height);

		float y_pos = y + padding;
		bool align_top = true;

		if (verticalAlignment == Alignment::End) {
			align_top = false;
			y_pos -= padding;
			if (0 <= height)
				y_pos += height;
		} else if (verticalAlignment == Alignment::Center) {
			y_pos -= padding;
			if (0 < lastTextHeight) {
				align_top = true;
				y_pos += (height - lastTextHeight) / 2;
			}
		}

		renderers.text.drawOnScreen(string, TextRenderOptions{
			.x = x,
			.y = y_pos,
			.scaleX = getTextScale(),
			.scaleY = getTextScale(),
			.wrapWidth = wrapped? 0 : getWrapWidth(width),
			.color = textColor,
			.alignTop = align_top,
			.shadow{0, 0, 0, 0},
		});
	}

	SizeRequestMode Label::getRequestMode() const {
		return SizeRequestMode::HeightForWidth;
	}

	void Label::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		if (orientation == Orientation::Horizontal) {
			minimum = 0;

			if (horizontalExpand && 0 < for_width) {
				natural = for_width;
			} else if (0 < for_width) {
				natural = std::min(for_width, renderers.text.textWidth(text, getTextScale()) + scale);
			} else {
				natural = renderers.text.textWidth(text, getTextScale()) + scale;
			}

			return;
		}

		// Add a little bit to account for descenders.
		const float addend = 2 * scale;

		if (lastTextHeight > 0 && for_width == lastRectangle.width) {
			minimum = natural = lastTextHeight + addend;
		} else if (for_width < 0) {
			minimum = 0;
			wrapped = text;
			natural = (lastTextHeight = renderers.text.textHeight(text, getTextScale())) + addend;
		} else {
			tryWrap(renderers.text, for_width);

			if (!wrapped) {
				minimum = natural = 0;
			} else {
				minimum = natural = (lastTextHeight = renderers.text.textHeight(wrapped.value(), getTextScale(), for_width)) + addend;
			}
		}

		if (verticalExpand) {
			natural = std::max(natural, for_height);
		}
	}

	void Label::setText(UString new_text) {
		if (text == new_text)
			return;

		text = std::move(new_text);
		wrapped.reset();
		lastUnwrappedTextWidth = ui.getRenderers().text.textWidth(text, getTextScale());
	}

	const UString & Label::getText() const {
		return text;
	}

	void Label::setTextColor(const Color &color) {
		textColor = color;
	}

	const Color & Label::getTextColor() const {
		return textColor;
	}

	float Label::getTextScale() const {
		return scale / 16;
	}

	float Label::getPadding() const {
		return scale * 2;
	}

	float Label::getWrapWidth(float width) const {
		return width;
	}

	void Label::tryWrap(const TextRenderer &texter, float width) {
		if (width <= 0) {
			wrapped.reset();
			return;
		}

		if (!wrapped) {
			wrapped = text.wrap(texter, getWrapWidth(width), getTextScale());
		}
	}
}
