#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/TextRenderer.h"
#include "ui/gl/widget/TooltipWidget.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

#include "threading/ThreadContext.h"

namespace {
	constexpr float DEFAULT_MAX_WIDTH = 800 * Game3::UI_SCALE / 8;
	constexpr Game3::Color DEFAULT_BACKGROUND_COLOR{"#000000af"};
	constexpr Game3::Color DEFAULT_TEXT_COLOR{"#ffffff"};
}

namespace Game3 {
	TooltipWidget::TooltipWidget(float scale):
		Widget(scale),
		maxWidth(DEFAULT_MAX_WIDTH),
		backgroundColor(DEFAULT_BACKGROUND_COLOR),
		textColor(DEFAULT_TEXT_COLOR) {}

	void TooltipWidget::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		if (!visible)
			return;

		if (region && !region->contains(x, y)) {
			hide();
			return;
		}

		Widget::render(ui, renderers, x, y, width, height);

		RectangleRenderer &rectangler = renderers.rectangle;
		TextRenderer &texter = renderers.text;

		const float padding = getPadding();
		const float text_scale = getTextScale();

		const float text_width = texter.textWidth(text, text_scale) + 2 * padding;
		const float effective_width = std::min(maxWidth, width < 0? text_width : std::min(width, text_width));

		const float text_height = texter.textHeight(text, text_scale, effective_width - 2 * padding) + 2 * padding;
		const float effective_height = height < 0? text_height : std::min(height, text_height);

		lastRectangle.width  = effective_width;
		lastRectangle.height = effective_height;

		rectangler(backgroundColor, Rectangle(x, y, effective_width, effective_height));

		const Rectangle interior(x, y, effective_width, effective_height);
		auto saver = ui.scissorStack.pushRelative(interior, renderers);

		texter(text, TextRenderOptions{
			.x = padding,
			.y = padding,
			.scaleX = text_scale,
			.scaleY = text_scale,
			.wrapWidth = static_cast<double>(interior.width - 2 * padding),
			.color = textColor,
			.alignTop = true,
			.shadow{0, 0, 0, 0},
		});
	}

	float TooltipWidget::calculateHeight(const RendererContext &, float, float available_height) {
		return available_height;
	}

	void TooltipWidget::hide() {
		visible = false;
		region.reset();
	}

	void TooltipWidget::hide(const Widget &updater) {
		if (lastUpdater.lock().get() == &updater) {
			hide();
		}
	}

	void TooltipWidget::show() {
		visible = true;
	}

	void TooltipWidget::show(const Widget &updater) {
		show();
		lastUpdater = updater.weak_from_this();
	}

	bool TooltipWidget::wasUpdatedBy(const Widget &widget) const {
		return lastUpdater.lock().get() == &widget;
	}

	void TooltipWidget::setText(Glib::ustring new_text) {
		text = std::move(new_text);
	}

	bool TooltipWidget::setText(Glib::ustring new_text, const Widget &updater) {
		return updateField(std::move(new_text), &TooltipWidget::text, updater);
	}

	void TooltipWidget::setMaxWidth(float new_max_width) {
		maxWidth = new_max_width;
	}

	bool TooltipWidget::setMaxWidth(float new_max_width, const Widget &updater) {
		return updateField(new_max_width, &TooltipWidget::maxWidth, updater);
	}

	void TooltipWidget::setBackgroundColor(const Color &new_color) {
		backgroundColor = new_color;
	}

	bool TooltipWidget::setBackgroundColor(const Color &new_color, const Widget &updater) {
		return updateField(new_color, &TooltipWidget::backgroundColor, updater);
	}

	void TooltipWidget::setTextColor(const Color &new_color) {
		textColor = new_color;
	}

	bool TooltipWidget::setTextColor(const Color &new_color, const Widget &updater) {
		return updateField(new_color, &TooltipWidget::textColor, updater);
	}

	void TooltipWidget::setRegion(std::optional<Rectangle> new_region) {
		region = std::move(new_region);
	}

	bool TooltipWidget::setRegion(std::optional<Rectangle> new_region, const Widget &updater) {
		return updateField(std::move(new_region), &TooltipWidget::region, updater);
	}

	float TooltipWidget::getTextScale() const {
		return scale / 16;
	}

	float TooltipWidget::getPadding() const {
		return scale * 4;
	}
}
