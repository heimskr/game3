#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "ui/gl/widget/BoxWidget.h"

namespace {
	constexpr float DEFAULT_SEPARATOR_THICKNESS = 1;
	constexpr float DEFAULT_PADDING = 2;
	constexpr Game3::Color DEFAULT_SEPARATOR_COLOR{"#49120060"};
}

namespace Game3 {
	BoxWidget::BoxWidget(float scale, Orientation orientation, float padding, float separator_thickness, Color separator_color):
		Widget(scale), orientation(orientation), padding(padding), separatorThickness(separator_thickness), separatorColor(separator_color) {
			if (orientation != Orientation::Vertical)
				throw std::invalid_argument("Non-vertical boxes are currently unsupported");
		}

	BoxWidget::BoxWidget(float scale, Orientation orientation):
		BoxWidget(scale, orientation, DEFAULT_PADDING, DEFAULT_SEPARATOR_THICKNESS, DEFAULT_SEPARATOR_COLOR) {}

	void BoxWidget::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		Widget::render(ui, renderers, x, y, width, height);

		// TODO: push to scissor stack, perhaps?

		assert(orientation == Orientation::Vertical);

		RectangleRenderer &rectangler = renderers.rectangle;

		lastRenderedHeight = 0;
		float original_y = y;

		for (WidgetPtr child = firstChild; child; child = child->getNextSibling()) {
			if (child != firstChild && separatorThickness > 0) {
				y += padding * scale;
				rectangler(separatorColor, x, y, width, separatorThickness * scale);
				y += (padding + separatorThickness) * scale;
				lastRenderedHeight += (2 * padding + separatorThickness) * scale;
				height -= (2 * padding + separatorThickness) * scale;
			}

			child->render(ui, renderers, x, y, width, height - (y - original_y));
			const float child_height = child->calculateHeight(renderers, width, height);
			y += child_height;
			lastRenderedHeight += child_height;
			height -= child_height;
		}
	}

	float BoxWidget::calculateHeight(const RendererContext &renderers, float available_width, float available_height) {
		if (childCount == 0)
			return 0;

		if (0 <= lastRenderedHeight)
			return lastRenderedHeight;

		float out = (childCount - 1) * scale * (2 * padding + separatorThickness);
		for (WidgetPtr child = firstChild; child; child = child->getNextSibling())
			out += child->calculateHeight(renderers, available_width, available_height);
		return out;
	}
}
