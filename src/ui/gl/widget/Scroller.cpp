#include "graphics/GL.h"
#include "graphics/Rectangle.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "ui/gl/widget/Scroller.h"
#include "ui/gl/UIContext.h"
#include "util/Defer.h"

namespace {
	constexpr float SCROLL_SPEED = 64;
	constexpr bool ALLOW_VERTICAL_OVERSCROLL = true;
	constexpr Game3::Color DEFAULT_SCROLLBAR_COLOR{"#49120080"};
}

namespace Game3 {
	Scroller::Scroller(UIContext &ui, float scale, Color scrollbar_color):
		Widget(ui, scale), scrollbarColor(scrollbar_color) {}

	Scroller::Scroller(UIContext &ui, float scale):
		Scroller(ui, scale, DEFAULT_SCROLLBAR_COLOR) {}

	void Scroller::render(const RendererContext &renderers, float x, float y, float width, float height) {
		float dummy{};

		if (firstChild && static_cast<int>(height) != lastRectangle.height) {
			firstChild->measure(renderers, Orientation::Vertical, width, height, dummy, lastChildHeight);
		}

		maybeRemeasure(renderers, width, height);

		Widget::render(renderers, x, y, width, height);

		if (!firstChild)
			return;

		auto saver = ui.scissorStack.pushRelative(Rectangle(x, y, width, height), renderers);

		firstChild->render(renderers, xOffset, yOffset, width, height);

		if (lastChildHeight > 0) {
			updateVerticalRectangle();
			const float vertical_fraction = height / (ALLOW_VERTICAL_OVERSCROLL? height + lastChildHeight : lastChildHeight);
			if (vertical_fraction < 1) {
				renderers.rectangle.drawOnScreen(scrollbarColor, *lastVerticalScrollbarRectangle - saver.rectangle);
			}
		}
	}

	bool Scroller::click(int button, int x, int y) {
		if (lastVerticalScrollMouse || lastHorizontalScrollMouse)
			return false;

		return firstChild && firstChild->click(button, x, y);
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

		if (firstChild && firstChild->dragStart(x, y))
			return true;

		lastVerticalScrollMouse = y - last_y;
		ui.addDragUpdater(shared_from_this());
		reverseScroll = true;
		return true;
	}

	bool Scroller::dragUpdate(int x, int y) {
		if (lastVerticalScrollMouse) {
			const auto start_y = *lastVerticalScrollMouse;
			const float last_y = lastRectangle.y;
			const float new_vertical_offset = getVerticalOffset() + (y - last_y - start_y) * (reverseScroll? -0.5 : 1.0);
			yOffset = fixYOffset(recalculateYOffset(new_vertical_offset));
			lastVerticalScrollMouse = y - last_y;
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

	bool Scroller::scroll(float x_delta, float y_delta, int, int) {
		xOffset += (getNatural()? x_delta : -x_delta) * SCROLL_SPEED;
		yOffset += (getNatural()? y_delta : -y_delta) * SCROLL_SPEED;
		xOffset = std::min(0.f, xOffset);
		yOffset = fixYOffset(yOffset);
		updateVerticalRectangle();
		static_assert(ALLOW_VERTICAL_OVERSCROLL); // TODO
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
		lastChildHeight = -1;
		lastVerticalScrollMouse.reset();
		lastHorizontalScrollMouse.reset();
		lastVerticalScrollbarRectangle.reset();
		lastHorizontalScrollbarRectangle.reset();
		Widget::clearChildren();
	}

	void Scroller::setChild(WidgetPtr new_child) {
		if (new_child)
			new_child->insertAtStart(shared_from_this());
		else if (firstChild)
			remove(firstChild);
		xOffset = 0;
		yOffset = 0;
	}

	bool Scroller::getNatural() const {
		return false;
	}

	float Scroller::getBarThickness() const {
		return 2 * scale;
	}

	float Scroller::getVerticalOffset() const {
		const float height = lastRectangle.height;
		const float vertical_fraction = height / (ALLOW_VERTICAL_OVERSCROLL? height + lastChildHeight : lastChildHeight);
		const float vertical_height = vertical_fraction * height;
		return yOffset / lastChildHeight * (vertical_height - height);
	}

	float Scroller::getHorizontalOffset() const {
		assert(false);
		return -1;
	}

	float Scroller::recalculateYOffset(float vertical_offset) const {
		const float height = lastRectangle.height;
		const float vertical_fraction = height / (ALLOW_VERTICAL_OVERSCROLL? height + lastChildHeight : lastChildHeight);
		const float vertical_height = vertical_fraction * height;
		return vertical_offset * lastChildHeight / (vertical_height - height);
	}

	float Scroller::recalculateXOffset(float) const {
		assert(false);
		return -1;
	}

	float Scroller::fixYOffset(float y_offset) const {
		y_offset = std::min(0.f, y_offset);
		if (lastChildHeight > 0)
			y_offset = std::max(y_offset, -lastChildHeight);
		return y_offset;
	}

	void Scroller::updateVerticalRectangle() {
		const auto [x, y, width, height] = lastRectangle;
		const float bar_thickness = getBarThickness();
		const float vertical_height = height * height / (ALLOW_VERTICAL_OVERSCROLL? height + lastChildHeight : lastChildHeight);
		lastVerticalScrollbarRectangle.emplace(x + width - bar_thickness, y + getVerticalOffset(), bar_thickness, vertical_height);
	}
}
