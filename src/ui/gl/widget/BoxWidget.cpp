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

		lastRenderedSize = {0, 0};
		float original_y = y;

		for (WidgetPtr child = firstChild; child; child = child->getNextSibling()) {
			if (child != firstChild && separatorThickness > 0) {
				y += padding * scale;
				rectangler(separatorColor, x, y, width, separatorThickness * scale);
				y += (padding + separatorThickness) * scale;
				lastRenderedSize.second += (2 * padding + separatorThickness) * scale;
				height -= (2 * padding + separatorThickness) * scale;
			}

			child->render(ui, renderers, x, y, width, height - (y - original_y));

			float child_minimum{};
			float child_natural{};
			child->measure(renderers, Orientation::Vertical, width, height, child_minimum, child_natural);

			y += child_natural;
			lastRenderedSize.second += child_natural;
			height -= child_natural;
		}
	}

	SizeRequestMode BoxWidget::getRequestMode() const {
		return orientation == Orientation::Horizontal? SizeRequestMode::WidthForHeight : SizeRequestMode::HeightForWidth;
	}

	void BoxWidget::measure(const RendererContext &renderers, Orientation measure_orientation, float for_width, float for_height, float &minimum, float &natural) {
		if (childCount == 0) {
			minimum = natural = 0;
			return;
		}

		assert(orientation == Orientation::Vertical); // TODO

		if (measure_orientation == orientation) {
			minimum = (childCount - 1) * scale * (2 * padding + separatorThickness);
			natural = minimum;

			for (WidgetPtr child = firstChild; child; child = child->getNextSibling()) {
				float child_minimum{};
				float child_natural{};
				child->measure(renderers, measure_orientation, for_width, for_height, child_minimum, child_natural);
				minimum += child_minimum;
				natural += child_natural;
			}
		} else {
			minimum = 0;
			natural = 0;

			for (WidgetPtr child = firstChild; child; child = child->getNextSibling()) {
				float child_minimum{};
				float child_natural{};
				child->measure(renderers, measure_orientation, for_width, for_height, child_minimum, child_natural);
				minimum = std::max(minimum, child_minimum);
				natural = std::max(natural, child_natural);
			}
		}
	}
}
