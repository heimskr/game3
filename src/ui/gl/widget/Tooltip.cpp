#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/TextRenderer.h"
#include "ui/gl/widget/Tooltip.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace {
	constexpr float DEFAULT_MAX_WIDTH = 800 * Game3::UI_SCALE / 8;
	constexpr Game3::Color DEFAULT_TOOLTIP_BACKGROUND_COLOR{"#000000af"};
	constexpr Game3::Color DEFAULT_TOOLTIP_TEXT_COLOR{"#ffffff"};
	constexpr float CURSOR_WIDTH = 10;
	constexpr float CURSOR_HEIGHT = 20;
}

namespace Game3 {
	Tooltip::Tooltip(UIContext &ui, float selfScale):
		Widget(ui, selfScale),
		maxWidth(DEFAULT_MAX_WIDTH),
		backgroundColor(DEFAULT_TOOLTIP_BACKGROUND_COLOR),
		textColor(DEFAULT_TOOLTIP_TEXT_COLOR) {}

	void Tooltip::render(const RendererContext &renderers, float x, float y, float width, float height) {
		if (!visible) {
			return;
		}

		if (positionOverride) {
			x = positionOverride->first;
			y = positionOverride->second;
		} else {
			if (region && !region->contains(x, y)) {
				hide();
				return;
			}

			x += CURSOR_WIDTH;
			y += CURSOR_HEIGHT;
		}

		Widget::render(renderers, x, y, width, height);

		RectangleRenderer &rectangler = renderers.rectangle;
		TextRenderer &texter = renderers.text;

		const auto scale = getScale();

		const float padding = getPadding();
		const float text_scale = getTextScale();

		const float text_width = texter.textWidth(text, text_scale) + 2 * padding + scale / 2;
		const float effective_width = std::min(maxWidth, width < 0? text_width : std::min(width, text_width));

		const float text_height = texter.textHeight(text, text_scale, effective_width - 2 * padding) + 2 * padding;
		const float effective_height = (height < 0? text_height : std::min(height, text_height));

		lastRectangle.width  = effective_width;
		lastRectangle.height = effective_height;

		if (x + effective_width > ui.getWidth() && x - effective_width >= 0) {
			lastRectangle.x -= effective_width;
			x -= effective_width;
		}

		const Rectangle interior(x, y, effective_width, effective_height);
		rectangler(backgroundColor, interior);
		auto saver = ui.scissorStack.pushRelative(interior, renderers);

		texter(text, TextRenderOptions{
			.x = padding,
			.y = padding,
			.scaleX = text_scale,
			.scaleY = text_scale,
			.wrapWidth = static_cast<double>(effective_width - 2 * padding),
			.color = textColor,
			.alignTop = true,
			.shadow{0, 0, 0, 0},
		});
	}

	SizeRequestMode Tooltip::getRequestMode() const {
		return SizeRequestMode::HeightForWidth;
	}

	void Tooltip::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		minimum = getPadding() * 2;

		if (orientation == Orientation::Horizontal) {
			natural = std::max(minimum, for_height);
		} else {
			natural = minimum + renderers.text.textHeight(text, getTextScale(), for_width - minimum);
		}
	}

	void Tooltip::hide() {
		visible = false;
		region.reset();
		positionOverride.reset();
	}

	void Tooltip::hide(TooltipUpdater &updater) {
		if (lastUpdater.lock().get() == &updater) {
			hide();
		}
	}

	void Tooltip::show() {
		visible = true;
	}

	bool Tooltip::wasUpdatedBy(const TooltipUpdater &updater) const {
		return lastUpdater.lock().get() == &updater;
	}

	void Tooltip::setText(UString new_text) {
		text = std::move(new_text);
	}

	void Tooltip::setMaxWidth(float new_max_width) {
		maxWidth = new_max_width;
	}

	void Tooltip::setBackgroundColor(const Color &new_color) {
		backgroundColor = new_color;
	}

	void Tooltip::setTextColor(const Color &new_color) {
		textColor = new_color;
	}

	void Tooltip::setRegion(std::optional<Rectangle> new_region) {
		region = std::move(new_region);
	}

	void Tooltip::setPositionOverride(std::optional<std::pair<float, float>> new_position_override) {
		positionOverride = std::move(new_position_override);
	}

	float Tooltip::getTextScale() const {
		return getScale() / 16;
	}

	float Tooltip::getPadding() const {
		return getScale() * 2;
	}
}
