#include "graphics/GL.h"
#include "graphics/Rectangle.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "ui/gl/widget/Scroller.h"
#include "ui/gl/UIContext.h"

namespace {
	constexpr float SCROLL_SPEED = 64;
	constexpr bool ALLOW_VERTICAL_OVERSCROLL = false;
	constexpr Game3::Color DEFAULT_SCROLLBAR_COLOR{"#49120080"};
}

namespace Game3 {
	Scroller::Scroller(UIContext &ui, float selfScale, Color scrollbar_color):
		Widget(ui, selfScale), scrollbarColor(scrollbar_color) {}

	Scroller::Scroller(UIContext &ui, float selfScale):
		Scroller(ui, selfScale, DEFAULT_SCROLLBAR_COLOR) {}

	void Scroller::render(const RendererContext &renderers, float x, float y, float width, float height) {
		float dummy{};

		if (firstChild && static_cast<int>(height) != lastRectangle.height) {
			firstChild->measure(renderers, Orientation::Vertical, width, height, dummy, lastChildHeight.emplace(-1));
		}

		// if (firstChild && static_cast<int>(width) != lastRectangle.width) {
		// 	firstChild->measure(renderers, Orientation::Horizontal, width, height, dummy, dummy);
		// }

		maybeRemeasure(renderers, width, height);

		Widget::render(renderers, x, y, width, height);

		if (!firstChild) {
			return;
		}

		auto saver = ui.scissorStack.pushRelative(Rectangle(x, y, width, height), renderers);

		if (!lastChildHeight) {
			firstChild->measure(renderers, Orientation::Vertical, width, height, dummy, lastChildHeight.emplace(-1));
		}

		firstChild->render(renderers, xOffset, yOffset, width, *lastChildHeight);

		if (lastChildHeight > 0) {
			updateVerticalRectangle();
			const float vertical_fraction = height / (ALLOW_VERTICAL_OVERSCROLL? height + lastChildHeight.value() : lastChildHeight.value());
			if (vertical_fraction < 1) {
				renderers.rectangle.drawOnScreen(scrollbarColor, *lastVerticalScrollbarRectangle - saver.rectangle);
			}
		}
	}

	bool Scroller::click(int button, int x, int y, Modifiers modifiers) {
		if (lastVerticalScrollMouse || lastHorizontalScrollMouse) {
			return false;
		}

		return firstChild && firstChild->click(button, x, y, modifiers);
	}

	bool Scroller::dragStart(int x, int y) {
		const auto [last_x, last_y, width, height] = lastRectangle;

		if (lastVerticalScrollbarRectangle) {
			if (lastVerticalScrollbarRectangle->contains(x, y)) {
				// Grab bar
				lastVerticalScrollMouse = y - last_y;
				ui.addDragUpdater(shared_from_this());
				return true;
			}

			const float bar_thickness = getBarThickness();
			if (Rectangle(last_x + width - bar_thickness, last_y, bar_thickness, height).contains(x, y)) {
				// Jump to clicked position
				const float new_vertical_offset = y - last_y - lastVerticalScrollbarRectangle->height / 2;
				yOffset = fixYOffset(recalculateYOffset(new_vertical_offset));
				lastVerticalScrollMouse = y - last_y;
				updateVerticalRectangle();
				ui.addDragUpdater(shared_from_this());
				return true;
			}
		}

		if (firstChild && firstChild->dragStart(x, y)) {
			return true;
		}

		lastVerticalScrollMouse = y - last_y;
		ui.addDragUpdater(shared_from_this());
		reverseScroll = true;
		return true;
	}

	bool Scroller::dragUpdate(int x, int y) {
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

	bool Scroller::dragEnd(int x, int y) {
		reverseScroll = false;

		if (lastVerticalScrollMouse) {
			lastVerticalScrollMouse.reset();
			return true;
		}

		if (lastHorizontalScrollMouse) {
			lastHorizontalScrollMouse.reset();
			return true;
		}

		return firstChild && firstChild->dragEnd(x, y);
	}

	bool Scroller::scroll(float x_delta, float y_delta, int, int, Modifiers) {
		xOffset += (getNatural()? -x_delta : x_delta) * SCROLL_SPEED;
		xOffset = std::min(0.f, xOffset);

		maybeRemeasureChildHeight();

		if (ALLOW_VERTICAL_OVERSCROLL || lastChildHeight > lastRectangle.height) {
			yOffset += (getNatural()? -y_delta : y_delta) * SCROLL_SPEED;
			yOffset = fixYOffset(yOffset);
		} else {
			yOffset = 0;
		}

		updateVerticalRectangle();
		// static_assert(ALLOW_VERTICAL_OVERSCROLL); // TODO
		return true;
	}

	SizeRequestMode Scroller::getRequestMode() const {
		return SizeRequestMode::Expansive;
	}

	void Scroller::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
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

		lastChildHeight.reset();
		return true;
	}

	void Scroller::childResized(const WidgetPtr &, int, int new_height) {
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
		lastChildHeight.reset();
	}

	bool Scroller::getNatural() const {
		return false;
	}

	float Scroller::getBarThickness() const {
		return 2 * selfScale;
	}

	float Scroller::getVerticalOffset() const {
		const float height = lastRectangle.height;
		const float vertical_fraction = replaceNaN(height / (ALLOW_VERTICAL_OVERSCROLL? height + lastChildHeight.value() : lastChildHeight.value()), 0);
		const float vertical_height = vertical_fraction * height;
		return replaceNaN(yOffset / lastChildHeight.value() * ((ALLOW_VERTICAL_OVERSCROLL? vertical_height : 0) - height), 0);
	}

	float Scroller::getHorizontalOffset() const {
		assert(false);
		return -1;
	}

	float Scroller::recalculateYOffset(float vertical_offset) const {
		const float height = lastRectangle.height;
		const float vertical_fraction = replaceNaN(height / (ALLOW_VERTICAL_OVERSCROLL? height + lastChildHeight.value() : lastChildHeight.value()), 0);
		const float vertical_height = vertical_fraction * height;
		return replaceNaN(vertical_offset * lastChildHeight.value() / ((ALLOW_VERTICAL_OVERSCROLL? vertical_height : 0) - height), 0);
	}

	float Scroller::recalculateXOffset(float) const {
		assert(false);
		return -1;
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

	void Scroller::updateVerticalRectangle() {
		const auto [x, y, width, height] = lastRectangle;
		const float vertical_height = replaceNaN(height * height / (ALLOW_VERTICAL_OVERSCROLL? height + lastChildHeight.value() : lastChildHeight.value()), 0);
		if (!std::isinf(vertical_height)) {
			const float bar_thickness = getBarThickness() * ui.scale;
			lastVerticalScrollbarRectangle.emplace(x + width - bar_thickness, y + getVerticalOffset(), bar_thickness, vertical_height);
		}
	}

	void Scroller::maybeRemeasureChildHeight() {
		if (!ALLOW_VERTICAL_OVERSCROLL && firstChild != nullptr && lastChildHeight.value() <= 0) {
			float dummy{};
			firstChild->measure(ui.getRenderers(), Orientation::Vertical, lastRectangle.width, lastRectangle.height, dummy, lastChildHeight.value());
		}
	}
}
