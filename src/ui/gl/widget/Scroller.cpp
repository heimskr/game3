#include "graphics/GL.h"
#include "math/Rectangle.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "ui/gl/widget/Scroller.h"
#include "ui/gl/UIContext.h"

namespace {
	constexpr float SCROLL_SPEED = 100;
	constexpr bool ALLOW_VERTICAL_OVERSCROLL = false;
	constexpr bool ALLOW_HORIZONTAL_OVERSCROLL = false;
	constexpr Game3::Color DEFAULT_SCROLLBAR_COLOR{"#49120080"};
}

namespace Game3 {
	Scroller::Scroller(UIContext &ui, float selfScale, Color scrollbar_color):
		Widget(ui, selfScale), scrollbarColor(scrollbar_color) {}

	Scroller::Scroller(UIContext &ui, float selfScale):
		Scroller(ui, selfScale, DEFAULT_SCROLLBAR_COLOR) {}

	void Scroller::render(const RendererContext &renderers, float x, float y, float width, float height) {
		float dummy{};

		if (firstChild && static_cast<int>(width) != lastRectangle.width) {
			firstChild->measure(renderers, Orientation::Horizontal, width, height, dummy, lastChildWidth.emplace(-1));
		}

		if (firstChild && static_cast<int>(height) != lastRectangle.height) {
			firstChild->measure(renderers, Orientation::Vertical, width, height, dummy, lastChildHeight.emplace(-1));
		}

		maybeRemeasure(renderers, width, height);

		Widget::render(renderers, x, y, width, height);

		if (!firstChild) {
			return;
		}

		auto saver = ui.scissorStack.pushRelative(Rectangle(x, y, width, height), renderers);

		if (!lastChildWidth) {
			firstChild->measure(renderers, Orientation::Horizontal, width, height, dummy, lastChildWidth.emplace(-1));
		}

		if (!lastChildHeight) {
			firstChild->measure(renderers, Orientation::Vertical, width, height, dummy, lastChildHeight.emplace(-1));
		}

		firstChild->render(renderers, xOffset, yOffset, width, *lastChildHeight);

		if (lastChildWidth > 0) {
			updateHorizontalRectangle();
			const float horizontal_fraction = width / (ALLOW_HORIZONTAL_OVERSCROLL? width + lastChildWidth.value() : lastChildWidth.value());
			if (horizontal_fraction < 1) {
				renderers.rectangle.drawOnScreen(scrollbarColor, *lastHorizontalScrollbarRectangle - saver.rectangle);
			}
		}

		if (lastChildHeight > 0) {
			updateVerticalRectangle();
			const float vertical_fraction = height / (ALLOW_VERTICAL_OVERSCROLL? height + lastChildHeight.value() : lastChildHeight.value());
			if (vertical_fraction < 1) {
				renderers.rectangle.drawOnScreen(scrollbarColor, *lastVerticalScrollbarRectangle - saver.rectangle);
			}
		}
	}

	bool Scroller::click(int button, int x, int y, Modifiers modifiers) {
		return firstChild && firstChild->click(button, x, y, modifiers);
	}

	bool Scroller::dragStart(int x, int y) {
		const auto [last_x, last_y, width, height] = lastRectangle;
		const float bar_thickness = getBarThickness() * ui.scale;
		bool updated = false;

		if (lastHorizontalScrollbarRectangle) {
			if (lastHorizontalScrollbarRectangle->contains(x, y)) {
				// Grab bar
				lastHorizontalScrollMouse = x - last_x;
				ui.addDragUpdater(shared_from_this());
				updated = true;
			} else if (Rectangle(last_x, last_y + height - bar_thickness, width, bar_thickness).contains(x, y)) {
				// Jump to clicked position
				const float new_horizontal_offset = x - last_x - lastHorizontalScrollbarRectangle->width / 2;
				xOffset = fixYOffset(recalculateYOffset(new_horizontal_offset));
				lastHorizontalScrollMouse = x - last_x;
				updateHorizontalRectangle();
				ui.addDragUpdater(shared_from_this());
				updated = true;
			}

			if (updated) {
				return true;
			}
		}

		if (lastVerticalScrollbarRectangle) {
			if (lastVerticalScrollbarRectangle->contains(x, y)) {
				// Grab bar
				lastVerticalScrollMouse = y - last_y;
				ui.addDragUpdater(shared_from_this());
				updated = true;
			} else if (Rectangle(last_x + width - bar_thickness, last_y, bar_thickness, height).contains(x, y)) {
				// Jump to clicked position
				const float new_vertical_offset = y - last_y - lastVerticalScrollbarRectangle->height / 2;
				yOffset = fixYOffset(recalculateYOffset(new_vertical_offset));
				lastVerticalScrollMouse = y - last_y;
				updateVerticalRectangle();
				ui.addDragUpdater(shared_from_this());
				updated = true;
			}

			if (updated) {
				return true;
			}
		}

		if (firstChild && firstChild->dragStart(x, y)) {
			return true;
		}

		lastHorizontalScrollMouse = x - last_x;
		lastVerticalScrollMouse = y - last_y;
		ui.addDragUpdater(shared_from_this());
		reverseScroll = true;
		return true;
	}

	bool Scroller::dragUpdate(int x, int y) {
		if (lastHorizontalScrollMouse) {
			maybeRemeasureChildWidth();

			if (ALLOW_HORIZONTAL_OVERSCROLL || lastChildWidth > lastRectangle.width) {
				const auto start_x = *lastHorizontalScrollMouse;
				const float last_x = lastRectangle.x;
				const float new_horizontal_offset = getHorizontalOffset() + (x - last_x - start_x) * (reverseScroll? -0.5 : 1.0);
				xOffset = fixXOffset(recalculateXOffset(new_horizontal_offset));
				lastHorizontalScrollMouse = x - last_x;
			} else {
				xOffset = 0;
				lastHorizontalScrollMouse = 0;
			}

			updateVerticalRectangle();
			return true;
		}

		if (lastVerticalScrollMouse) {
			maybeRemeasureChildHeight();

			if (ALLOW_VERTICAL_OVERSCROLL || lastChildHeight > lastRectangle.height) {
				const auto start_y = *lastVerticalScrollMouse;
				const float last_y = lastRectangle.y;
				const float new_vertical_offset = getVerticalOffset() + (y - last_y - start_y) * (reverseScroll? -0.5 : 1.0);
				yOffset = fixYOffset(recalculateYOffset(new_vertical_offset));
				lastVerticalScrollMouse = y - last_y;
			} else {
				yOffset = 0;
				lastVerticalScrollMouse = 0;
			}

			updateVerticalRectangle();
			return true;
		}

		return firstChild && firstChild->dragUpdate(x, y);
	}

	bool Scroller::dragEnd(int x, int y, double displacement) {
		reverseScroll = false;

		if (lastHorizontalScrollMouse) {
			lastHorizontalScrollMouse.reset();
			return true;
		}

		if (lastVerticalScrollMouse) {
			lastVerticalScrollMouse.reset();
			return true;
		}

		return firstChild && firstChild->dragEnd(x, y, displacement);
	}

	bool Scroller::scroll(float x_delta, float y_delta, int, int, Modifiers) {
		maybeRemeasureChildWidth();

		if (ALLOW_HORIZONTAL_OVERSCROLL || lastChildWidth > lastRectangle.width) {
			xOffset += (getNatural()? -x_delta : x_delta) * SCROLL_SPEED;
			xOffset = fixXOffset(xOffset);
		} else {
			xOffset = 0;
		}

		updateHorizontalRectangle();

		maybeRemeasureChildHeight();

		if (ALLOW_VERTICAL_OVERSCROLL || lastChildHeight > lastRectangle.height) {
			yOffset += (getNatural()? -y_delta : y_delta) * SCROLL_SPEED;
			yOffset = fixYOffset(yOffset);
		} else {
			yOffset = 0;
		}

		updateVerticalRectangle();
		return true;
	}

	SizeRequestMode Scroller::getRequestMode() const {
		return SizeRequestMode::Expansive;
	}

	void Scroller::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		if (firstChild) {
			if (orientation == Orientation::Horizontal) {
				if (getHorizontalExpand() == Expansion::Shrink) {
					firstChild->measure(renderers, orientation, for_width, for_height, minimum, natural);
					minimum = std::min(minimum, for_width);
					natural = std::min(natural, for_width);
					return;
				}
			} else if (getVerticalExpand() == Expansion::Shrink) {
				firstChild->measure(renderers, orientation, for_width, for_height, minimum, natural);
				minimum = std::min(minimum, for_height);
				natural = std::min(natural, for_height);
				return;
			}
		}

		WidgetPtr child = firstChild;

		natural = orientation == Orientation::Horizontal? for_width : for_height;

		if (!child) {
			minimum = natural;
			return;
		}

		float dummy{};
		child->measure(renderers, orientation, for_width, for_height, minimum, dummy);
		minimum = std::min(minimum, natural);
	}

	void Scroller::clearChildren() {
		xOffset = 0;
		yOffset = 0;
		lastChildWidth.reset();
		lastChildHeight.reset();
		lastVerticalScrollMouse.reset();
		lastHorizontalScrollMouse.reset();
		lastVerticalScrollbarRectangle.reset();
		lastHorizontalScrollbarRectangle.reset();
		Widget::clearChildren();
	}

	bool Scroller::onChildrenUpdated() {
		if (!Widget::onChildrenUpdated()) {
			return false;
		}

		lastChildWidth.reset();
		lastChildHeight.reset();
		return true;
	}

	void Scroller::childResized(const WidgetPtr &, Orientation, int new_width, int new_height) {
		lastChildWidth = new_width;
		lastChildHeight = new_height;
	}

	void Scroller::setChild(WidgetPtr new_child) {
		if (new_child == firstChild) {
			return;
		}

		if (new_child) {
			new_child->insertAtStart(shared_from_this());
		} else if (firstChild) {
			remove(firstChild);
		}

		xOffset = 0;
		yOffset = 0;
		lastChildWidth.reset();
		lastChildHeight.reset();
	}

	float Scroller::getHorizontalOffset() const {
		if (!lastChildWidth) {
			return 0;
		}

		const float width = lastRectangle.width;
		const float horizontal_fraction = replaceNaN(width / (ALLOW_HORIZONTAL_OVERSCROLL? width + lastChildWidth.value() : lastChildWidth.value()), 0);
		const float horizontal_width = horizontal_fraction * width;
		return replaceNaN(xOffset / lastChildWidth.value() * ((ALLOW_HORIZONTAL_OVERSCROLL? horizontal_width : 0) - width), 0);
	}

	float Scroller::getVerticalOffset() const {
		if (!lastChildHeight) {
			return 0;
		}

		const float height = lastRectangle.height;
		const float vertical_fraction = replaceNaN(height / (ALLOW_VERTICAL_OVERSCROLL? height + lastChildHeight.value() : lastChildHeight.value()), 0);
		const float vertical_height = vertical_fraction * height;
		return replaceNaN(yOffset / lastChildHeight.value() * ((ALLOW_VERTICAL_OVERSCROLL? vertical_height : 0) - height), 0);
	}

	void Scroller::scrollToTop() {
		yOffset = 0;
	}

	bool Scroller::getNatural() const {
		return false;
	}

	float Scroller::getBarThickness() const {
		return 2 * selfScale;
	}

	float Scroller::recalculateXOffset(float horizontal_offset) const {
		const float width = lastRectangle.width;
		const float horizontal_fraction = replaceNaN(width / (ALLOW_HORIZONTAL_OVERSCROLL? width + lastChildWidth.value() : lastChildWidth.value()), 0);
		const float horizontal_width = horizontal_fraction * width;
		return replaceNaN(horizontal_offset * lastChildWidth.value() / ((ALLOW_HORIZONTAL_OVERSCROLL? horizontal_width : 0) - width), 0);
	}

	float Scroller::recalculateYOffset(float vertical_offset) const {
		const float height = lastRectangle.height;
		const float vertical_fraction = replaceNaN(height / (ALLOW_VERTICAL_OVERSCROLL? height + lastChildHeight.value() : lastChildHeight.value()), 0);
		const float vertical_height = vertical_fraction * height;
		return replaceNaN(vertical_offset * lastChildHeight.value() / ((ALLOW_VERTICAL_OVERSCROLL? vertical_height : 0) - height), 0);
	}

	float Scroller::fixXOffset(float x_offset) const {
		x_offset = std::min(0.f, x_offset);
		if constexpr (ALLOW_HORIZONTAL_OVERSCROLL) {
			if (lastChildWidth.value() > 0) {
				x_offset = std::max(x_offset, -lastChildWidth.value());
			}
		} else {
			if (lastChildWidth.value() - lastRectangle.width > 0) {
				x_offset = std::max(x_offset, lastRectangle.width - lastChildWidth.value());
			}
		}
		return x_offset;
	}

	float Scroller::fixYOffset(float y_offset) const {
		y_offset = std::min(0.f, y_offset);
		if constexpr (ALLOW_VERTICAL_OVERSCROLL) {
			if (lastChildHeight.value() > 0) {
				y_offset = std::max(y_offset, -lastChildHeight.value());
			}
		} else {
			if (lastChildHeight.value() - lastRectangle.height > 0) {
				y_offset = std::max(y_offset, lastRectangle.height - lastChildHeight.value());
			}
		}
		return y_offset;
	}

	void Scroller::updateHorizontalRectangle() {
		const auto [x, y, width, height] = lastRectangle;
		const float horizontal_width = replaceNaN(width * width / (ALLOW_VERTICAL_OVERSCROLL? width + lastChildWidth.value() : lastChildWidth.value()), 0);
		if (!std::isinf(horizontal_width)) {
			const float bar_thickness = getBarThickness() * ui.scale;
			lastHorizontalScrollbarRectangle.emplace(x + getHorizontalOffset(), y + height - bar_thickness, horizontal_width, bar_thickness);
		}
	}

	void Scroller::updateVerticalRectangle() {
		const auto [x, y, width, height] = lastRectangle;
		const float vertical_height = replaceNaN(height * height / (ALLOW_VERTICAL_OVERSCROLL? height + lastChildHeight.value() : lastChildHeight.value()), 0);
		if (!std::isinf(vertical_height)) {
			const float bar_thickness = getBarThickness() * ui.scale;
			lastVerticalScrollbarRectangle.emplace(x + width - bar_thickness, y + getVerticalOffset(), bar_thickness, vertical_height);
		}
	}

	void Scroller::maybeRemeasureChildWidth() {
		if (!ALLOW_HORIZONTAL_OVERSCROLL && firstChild != nullptr && lastChildWidth.value() <= 0) {
			float dummy{};
			firstChild->measure(ui.getRenderers(0), Orientation::Horizontal, lastRectangle.width, lastRectangle.height, dummy, lastChildWidth.value());
		}
	}

	void Scroller::maybeRemeasureChildHeight() {
		if (!ALLOW_VERTICAL_OVERSCROLL && firstChild != nullptr && lastChildHeight.value() <= 0) {
			float dummy{};
			firstChild->measure(ui.getRenderers(0), Orientation::Vertical, lastRectangle.width, lastRectangle.height, dummy, lastChildHeight.value());
		}
	}
}
