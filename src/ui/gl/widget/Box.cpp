#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "ui/gl/widget/Box.h"

namespace {
	constexpr float DEFAULT_SEPARATOR_THICKNESS = 1;
	constexpr float DEFAULT_PADDING = 2;
	constexpr Game3::Color DEFAULT_SEPARATOR_COLOR{"#49120060"};
}

namespace Game3 {
	Box::Box(float scale, Orientation orientation, float padding, float separator_thickness, Color separator_color):
		Widget(scale),
		orientation(orientation),
		padding(padding),
		separatorThickness(separator_thickness),
		separatorColor(separator_color) {}

	Box::Box(float scale, Orientation orientation):
		Box(scale, orientation, DEFAULT_PADDING, DEFAULT_SEPARATOR_THICKNESS, DEFAULT_SEPARATOR_COLOR) {}

	void Box::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		Widget::render(ui, renderers, x, y, width, height);

		const bool vertical = orientation == Orientation::Vertical;

		RectangleRenderer &rectangler = renderers.rectangle;
		lastRenderedSize = {0, 0};

		const float separator_width = vertical? width : separatorThickness * scale;
		const float separator_height = vertical? separatorThickness * scale : height;
		float &coordinate = vertical? y : x;
		float &size = vertical? height : width;
		float &last_size = vertical? lastRenderedSize.second : lastRenderedSize.first;
		const float original_width = width;
		const float original_height = height;

		size_t i = 0;

		for (WidgetPtr child = firstChild; child; child = child->getNextSibling()) {
			if (child != firstChild) {
				if (separatorThickness > 0) {
					coordinate += padding * scale;
					rectangler(separatorColor, x, y, separator_width, separator_height);
					coordinate += (padding + separatorThickness) * scale;
					last_size += (2 * padding + separatorThickness) * scale;
					size -= (2 * padding + separatorThickness) * scale;
				} else {
					coordinate += padding * scale;
					last_size += 2 * padding * scale;
					size -= 2 * padding * scale;
				}
			}

			float to_add = -1;

			bool try_measure = true;

			if (i < childMeasurements.size()) {
				const auto &[child_width, child_height] = childMeasurements[i];
				to_add = vertical? child_height : child_width;

				if (to_add < 0) {
					float minimum{}, natural{};
					measure(renderers, orientation, original_width, original_height, minimum, natural);
					to_add = vertical? child_height : child_width;
					assert(0 <= to_add);
				}

				if (0 <= to_add) {
					try_measure = false;
					child->render(ui, renderers, x, y, child_width < 0? width : child_width, child_height < 0? height : child_height);
				}
			}

			if (try_measure) {
				float child_minimum{};
				float child_natural{};
				child->measure(renderers, orientation, width, height, child_minimum, child_natural);

				if (vertical)
					child->render(ui, renderers, x, y, width, child_natural);
				else
					child->render(ui, renderers, x, y, child_natural, height);

				to_add = child_natural;
			}

			assert(0 <= to_add);

			coordinate += to_add;
			last_size += to_add;
			size -= to_add;

			++i;
		}
	}

	SizeRequestMode Box::getRequestMode() const {
		return orientation == Orientation::Horizontal? SizeRequestMode::WidthForHeight : SizeRequestMode::HeightForWidth;
	}

	void Box::measure(const RendererContext &renderers, Orientation measure_orientation, float for_width, float for_height, float &minimum, float &natural) {
		if (childCount == 0) {
			minimum = natural = 0;
			return;
		}

		size_t i = 0;
		childMeasurements.resize(childCount, {-1, -1});

		if (measure_orientation == orientation) {
			minimum = natural = (childCount - 1) * scale * (2 * padding + separatorThickness);
			const float original_minimum = minimum;

			float accumulated_nonexpanding_natural = 0;
			std::size_t expand_count = 0;
			std::size_t j = 0;
			std::vector<float> naturals(childCount, -1);

			for (WidgetPtr child = firstChild; child; child = child->getNextSibling()) {
				if (child->getExpand(measure_orientation)) {
					++expand_count;
				} else {
					float child_minimum{};
					float child_natural{};
					child->measure(renderers, measure_orientation, for_width, for_height, child_minimum, child_natural);
					minimum += child_minimum;
					natural += child_natural;
					accumulated_nonexpanding_natural += child_natural;
					naturals[j] = child_natural;
				}

				++j;
			}

			j = 0;

			const float for_size = measure_orientation == Orientation::Horizontal? for_width : for_height;
			if (0 < expand_count)
				natural = for_size;

			for (WidgetPtr child = firstChild; child; child = child->getNextSibling()) {
				auto &pair = childMeasurements.at(i++);
				auto &item = measure_orientation == Orientation::Horizontal? pair.first : pair.second;

				if (child->getExpand(measure_orientation)) {
					item = (for_size - accumulated_nonexpanding_natural - original_minimum) / expand_count;
				} else {
					item = naturals[j];
				}

				++j;
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
				auto &pair = childMeasurements.at(i++);
				(measure_orientation == Orientation::Horizontal? pair.first : pair.second) = child_natural;
			}
		}
	}

	void Box::setPadding(float new_padding) {
		padding = new_padding;
	}

	void Box::setSeparatorThickness(float new_separator_thickness) {
		separatorThickness = new_separator_thickness;
	}

	void Box::setSeparatorColor(Color new_separator_color) {
		separatorColor = new_separator_color;
	}
}
