#include "graphics/GL.h"
#include "graphics/Rectangle.h"
#include "graphics/RendererContext.h"
#include "ui/gl/widget/ScrollerWidget.h"
#include "ui/gl/UIContext.h"
#include "util/Defer.h"

namespace {
	constexpr float SCROLL_SPEED = 48;
}

namespace Game3 {
	void ScrollerWidget::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		Widget::render(ui, renderers, x, y, width, height);

		if (!child)
			return;

		ui.scissorStack.pushRelative({Rectangle(x, y, width, height), false});
		Defer pop([&] { ui.scissorStack.pop(); });

		child->render(ui, renderers, xOffset, yOffset, width, height);
	}

	bool ScrollerWidget::click(UIContext &ui, int button, int x, int y) {
		return child && child->click(ui, button, x, y);
	}

	bool ScrollerWidget::dragStart(UIContext &ui, int x, int y) {
		return child && child->dragStart(ui, x, y);
	}

	bool ScrollerWidget::dragUpdate(UIContext &ui, int x, int y) {
		return child && child->dragUpdate(ui, x, y);
	}

	bool ScrollerWidget::dragEnd(UIContext &ui, int x, int y) {
		return child && child->dragEnd(ui, x, y);
	}

	bool ScrollerWidget::scroll(UIContext &ui, float x_delta, float y_delta, int, int) {
		xOffset += (getNatural()? x_delta : -x_delta) * SCROLL_SPEED;
		yOffset += (getNatural()? y_delta : -y_delta) * SCROLL_SPEED;
		xOffset = std::min(0.f, xOffset);
		yOffset = std::min(0.f, yOffset);
		if (child)
			yOffset = std::max(yOffset, -child->calculateHeight(ui.getRenderers(), lastWidth, lastHeight));
		return true;
	}

	float ScrollerWidget::calculateHeight(const RendererContext &, float, float available_height) {
		return available_height;
	}

	void ScrollerWidget::setChild(WidgetPtr new_child) {
		child = std::move(new_child);
		xOffset = 0;
		yOffset = 0;
	}

	bool ScrollerWidget::getNatural() const {
		return false;
	}
}
