#include "ui/Canvas.h"
#include "graphics/RendererContext.h"
#include "graphics/TextRenderer.h"
#include "graphics/Texture.h"
#include "ui/gl/widget/LabelWidget.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	LabelWidget::LabelWidget(float scale):
		Widget(scale) {}

	void LabelWidget::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		if (lastRectangle.width != width)
			wrapped.reset();

		Widget::render(ui, renderers, x, y, width, height);
		const float padding = getPadding();

		tryWrap(renderers.text, width);
		const UString &string = wrapped? wrapped.value() : text;

		renderers.text.drawOnScreen(string, TextRenderOptions{
			.x = x,
			.y = y + padding,
			.scaleX = getTextScale(),
			.scaleY = getTextScale(),
			.wrapWidth = wrapped? 0 : getWrapWidth(width),
			.color{0, 0, 0, 1},
			.alignTop = true,
			.shadow{0, 0, 0, 0},
			.heightOut = &lastTextHeight,
		});

		lastTextHeight += padding;
	}

	SizeRequestMode LabelWidget::getRequestMode() const {
		return SizeRequestMode::HeightForWidth;
	}

	void LabelWidget::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		if (orientation == Orientation::Horizontal) {
			minimum = 0;
			if (for_width < 0)
				natural = renderers.text.textWidth(text, getTextScale());
			else
				natural = for_width;
			return;
		}

		// Add a little bit to account for descenders.
		const float addend = 2 * scale;

		if (lastTextHeight > 0 && for_width == lastRectangle.width) {
			minimum = natural = lastTextHeight + addend;
			return;
		}

		if (for_width < 0) {
			minimum = 0;
			wrapped = text;
			natural = (lastTextHeight = renderers.text.textHeight(text, getTextScale())) + addend;
			return;
		}

		tryWrap(renderers.text, for_width);

		if (!wrapped) {
			minimum = natural = 0;
			return;
		}

		minimum = natural = (lastTextHeight = renderers.text.textHeight(wrapped.value(), getTextScale(), for_width)) + addend;
	}

	void LabelWidget::setText(UIContext &ui, UString new_text) {
		if (text == new_text)
			return;

		text = std::move(new_text);
		wrapped.reset();
		tryWrap(ui.canvas.textRenderer, lastRectangle.width);
	}

	const UString & LabelWidget::getText() const {
		return text;
	}

	float LabelWidget::getTextScale() const {
		return scale / 16;
	}

	float LabelWidget::getPadding() const {
		return scale * 2;
	}

	float LabelWidget::getWrapWidth(float width) const {
		return width;
	}

	void LabelWidget::tryWrap(const TextRenderer &texter, float width) {
		if (width <= 0) {
			wrapped.reset();
			return;
		}

		if (!wrapped) {
			wrapped = text.wrap(texter, getWrapWidth(width), getTextScale());
		}
	}
}
