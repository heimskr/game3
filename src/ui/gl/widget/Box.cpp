#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "ui/gl/widget/Box.h"

namespace {
	constexpr float DEFAULT_SEPARATOR_THICKNESS = 1;
	constexpr float DEFAULT_PADDING = 2;
}

namespace Game3 {
	Box::Box(UIContext &ui, float selfScale, Orientation orientation, float padding, float separator_thickness, Color separator_color):
		ChildDependentExpandingWidget<Widget>(ui, selfScale),
		orientation(orientation),
		padding(padding),
		separatorThickness(separator_thickness),
		separatorColor(separator_color) {}

	Box::Box(UIContext &ui, float selfScale, Orientation orientation, float padding):
		Box(ui, selfScale, orientation, padding, padding == 0? 0 : DEFAULT_SEPARATOR_THICKNESS, padding == 0? Color{} : DEFAULT_SEPARATOR_COLOR) {}

	Box::Box(UIContext &ui, float selfScale, Orientation orientation):
		Box(ui, selfScale, orientation, DEFAULT_PADDING, DEFAULT_SEPARATOR_THICKNESS, DEFAULT_SEPARATOR_COLOR) {}

	void Box::render(const RendererContext &renderers, float x, float y, float width, float height) {
		if (width < -1 || height < -1) {
			ChildDependentExpandingWidget<Widget>::render(renderers, x, y, width, height);
			return;
		}

		maybeRemeasure(renderers, width, height);
		ChildDependentExpandingWidget<Widget>::render(renderers, x, y, width, height);

		if (shouldCull()) {
			return;
		}

		RectangleRenderer &rectangler = renderers.rectangle;

		const auto scale = getScale();

		const bool vertical = orientation == Orientation::Vertical;
		const float separator_width = vertical? width : separatorThickness * scale;
		const float separator_height = vertical? separatorThickness * scale : height;
		float &coordinate = vertical? y : x;
		float &size = vertical? height : width;

		std::size_t i = 0;

		for (WidgetPtr child = firstChild; child; child = child->getNextSibling()) {
			if (child != firstChild) {
				if (separatorThickness > 0) {
					coordinate += padding * scale;
					rectangler(separatorColor, x, y, separator_width, separator_height);
					coordinate += (padding + separatorThickness) * scale;
					size -= (2 * padding + separatorThickness) * scale;
				} else {
					coordinate += padding * scale;
					size -= padding * scale;
				}
			}

			float to_add = -1;

			bool try_measure = true;

			if (i < childMeasurements.size()) {
				const auto &pair = childMeasurements.at(i);
				auto [child_width, child_height] = pair;
				to_add = vertical? child_height : child_width;

				if (to_add < 0) {
					float minimum{}, natural{};
					measure(renderers, orientation, width, height, minimum, natural);
					// Resize can invalidate. That shouldn't happen here, but just in case...
					const auto &pair = childMeasurements.at(i);
					child_width = pair.first;
					child_height = pair.second;
					to_add = vertical? child_height : child_width;
					if (to_add < 0) {
						// ERR("[Box.cpp:{}] to_add for {} in {} is {}", __LINE__, child->describe(), describe(), to_add);
						// assert(0 <= to_add);
					}
				}

				if (0 <= to_add) {
					try_measure = false;
					if (orientation == Orientation::Horizontal) {
						child->render(renderers, x, y, child_width < 0? width : child_width, maximumPerpendicularChildMeasurement.value_or(child_height < 0? height : child_height));
					} else {
						child->render(renderers, x, y, maximumPerpendicularChildMeasurement.value_or(child_width < 0? width : child_width), child_height < 0? height : child_height);
					}
				}
			}

			if (try_measure) {
				float child_minimum{};
				float child_natural{};
				child->measure(renderers, orientation, width, height, child_minimum, child_natural);

				if (vertical) {
					child->render(renderers, x, y, width, child_natural);
				} else {
					child->render(renderers, x, y, child_natural, height);
				}

				to_add = child_natural;
			}

			if (to_add < 0) {
				ERR("[Box.cpp:{}] to_add for {} in {} is {}", __LINE__, child->describe(), describe(), to_add);
				to_add = 0;
				// assert(0 <= to_add);
			}

			coordinate += to_add;
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

		std::size_t i = 0;
		childMeasurements.resize(childCount, {-1, -1});

		if (measure_orientation == orientation) {
			minimum = natural = (childCount - 1) * getScale() * (0 < separatorThickness? 2 * padding + separatorThickness : padding);
			const float original_minimum = minimum;

			float accumulated_nonexpanding_natural = 0;
			std::size_t expand_count = 0;
			std::size_t j = 0;
			std::vector<float> naturals(childCount, -1);

			// const bool vertical = measure_orientation == Orientation::Vertical;

			for (WidgetPtr child = firstChild; child; child = child->getNextSibling()) {
				if (child->getExpand(measure_orientation) == Expansion::Expand) {
					++expand_count;
				} else {
					float child_minimum{};
					float child_natural{};
					// child->measure(renderers, measure_orientation, vertical? for_width : -1, vertical? -1 : for_height, child_minimum, child_natural);
					// child->measure(renderers, measure_orientation, -1, -1, child_minimum, child_natural);
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

			if (0 < expand_count) {
				natural = for_size;
			}

			for (WidgetPtr child = firstChild; child; child = child->getNextSibling()) {
				auto &pair = childMeasurements.at(j);
				auto &item = measure_orientation == Orientation::Horizontal? pair.first : pair.second;

				if (child->getExpand(measure_orientation) == Expansion::Expand) {
					item = (for_size - accumulated_nonexpanding_natural - original_minimum) / expand_count;
				} else {
					item = naturals[j];
				}

				++j;
			}
		} else {
			maximumPerpendicularChildMeasurement.reset();

			minimum = 0;
			natural = 0;

			bool expansive = false;

			for (WidgetPtr child = firstChild; child; child = child->getNextSibling()) {
				float child_minimum{};
				float child_natural{};
				const bool child_expansive = child->getExpand(measure_orientation) == Expansion::Expand;
				child->measure(renderers, measure_orientation, for_width, for_height, child_minimum, child_natural);
				minimum = std::max(minimum, child_minimum);
				natural = std::max(natural, child_natural);

				if (child_expansive) {
					expansive = true;
					child_natural = measure_orientation == Orientation::Horizontal? for_width : for_height;
				}

				if (0 <= child_natural) {
					maximumPerpendicularChildMeasurement = std::max(child_natural, maximumPerpendicularChildMeasurement.value_or(-1));
				}

				auto &[child_width, child_height] = childMeasurements.at(i);
				(measure_orientation == Orientation::Horizontal? child_width : child_height) = child_natural;

				++i;
			}

			if (expansive) {
				natural = std::max(natural, measure_orientation == Orientation::Vertical? for_height : for_width);
			}
		}

		if (measure_orientation == Orientation::Horizontal) {
			setLastWidth(natural);
		} else {
			setLastHeight(natural);
		}
	}

	bool Box::onChildrenUpdated() {
		if (!ChildDependentExpandingWidget<Widget>::onChildrenUpdated()) {
			return false;
		}

		childMeasurements.clear();
		lastRectangle.width = -1;
		lastRectangle.height = -1;
		return true;
	}

	void Box::append(const WidgetPtr &child) {
		child->insertAtEnd(shared_from_this());
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
