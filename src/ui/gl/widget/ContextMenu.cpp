#include "util/Log.h"
#include "ui/gl/widget/ContextMenu.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	ContextMenu::ContextMenu(UIContext &ui, float scale, WidgetPtr anchor, float x_offset, float y_offset):
		Box(ui, scale, Orientation::Vertical, 1, 0.5),
		anchor(std::move(anchor)),
		xOffset(x_offset),
		yOffset(y_offset) {}

	void ContextMenu::render(const RendererContext &renderers, float x, float y, float, float) {
		const Rectangle anchor_rectangle = anchor->getLastRectangle();

		x = anchor_rectangle.x + xOffset;
		y = anchor_rectangle.y + yOffset;

		const auto scale = getScale();

		{
			const float frame_scale = scale / 2;
			auto saver = ui.scissorStack.pushAbsolute(Rectangle(x - 6 * frame_scale, y - 6 * frame_scale, lastWidth + 12 * frame_scale, lastHeight + 12 * frame_scale), renderers);
			ui.drawFrame(renderers, frame_scale, true, FRAME_PIECES, DEFAULT_BACKGROUND_COLOR);
		}

		lastRectangle.x = x;
		lastRectangle.y = y;

		Box::render(renderers, x, y, lastWidth, lastHeight);
	}

	void ContextMenu::measure(const RendererContext &renderers, Orientation measure_orientation, float for_width, float for_height, float &minimum, float &natural) {
		Box::measure(renderers, measure_orientation, for_width, for_height, minimum, natural);

		if (measure_orientation == Orientation::Vertical) {
			lastHeight = natural;
		} else {
			lastWidth = natural;
		}
	}

	bool ContextMenu::blocksMouse(int, int, bool is_drag_update) const {
		return !is_drag_update;
	}

	bool ContextMenu::keyPressed(uint32_t key, Modifiers, bool) {
		if (key == GLFW_KEY_ESCAPE) {
			ui.setContextMenu(nullptr);
			return true;
		}

		return false;
	}

	void ContextMenu::addItem(std::shared_ptr<ContextMenuItem> item) {
		append(item);
		items.emplace_back(std::move(item));
	}

	WidgetPtr ContextMenu::getAnchor() const {
		return anchor;
	}

	float ContextMenu::getXOffset() const {
		return xOffset;
	}

	float ContextMenu::getYOffset() const {
		return yOffset;
	}

	void ContextMenu::setXOffset(float offset) {
		xOffset = offset;
	}

	void ContextMenu::setYOffset(float offset) {
		yOffset = offset;
	}

	void ContextMenu::adjustPosition() {
		Rectangle bounds = getLastRectangle();

		if (anchor) {
			Rectangle anchor_position = anchor->getLastRectangle();
			bounds.x = getXOffset() + anchor_position.x;
			bounds.y = getYOffset() + anchor_position.y;
		} else {
			bounds.x = getXOffset();
			bounds.y = getYOffset();
		}

		if (bounds.x + bounds.width > ui.getWidth()) {
			xOffset -= bounds.width;
		}

		if (bounds.y + bounds.height > ui.getHeight()) {
			yOffset -= bounds.height;
		}
	}

	ContextMenuItem::ContextMenuItem(UIContext &ui, float scale, UString text, std::function<void()> onSelect):
		Label(ui, scale),
		onSelect(std::move(onSelect)) {
			setText(std::move(text));
			setOnClick([this](Widget &, int button, int, int) {
				if (button != LEFT_BUTTON) {
					return false;
				}
				this->onSelect();
				this->ui.setContextMenu(nullptr); // TODO: is this safe?
				return true;
			});
		}
}
