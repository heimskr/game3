#include "Log.h"
#include "ui/gl/widget/ContextMenu.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	ContextMenu::ContextMenu(UIContext &ui, float scale, WidgetPtr anchor, float x_offset, float y_offset):
		Box(ui, scale, Orientation::Vertical),
		anchor(std::move(anchor)),
		xOffset(x_offset),
		yOffset(y_offset) {}

	void ContextMenu::render(const RendererContext &renderers, float x, float y, float width, float height) {
		const Rectangle anchor_rectangle = anchor->getLastRectangle();

		x = anchor_rectangle.x + xOffset;
		y = anchor_rectangle.y + yOffset;

		{
			const float frame_scale = scale / 2;
			auto saver = ui.scissorStack.pushAbsolute(Rectangle(x - 6 * frame_scale, y - 6 * frame_scale, lastWidth + 12 * frame_scale, lastHeight + 12 * frame_scale), renderers);
			ui.drawFrame(renderers, frame_scale, true, FRAME_PIECES, DEFAULT_BACKGROUND_COLOR);
		}

		Box::render(renderers, x, y, width, height);
	}

	void ContextMenu::measure(const RendererContext &renderers, Orientation measure_orientation, float for_width, float for_height, float &minimum, float &natural) {
		Box::measure(renderers, measure_orientation, for_width, for_height, minimum, natural);

		if (measure_orientation == Orientation::Vertical) {
			lastHeight = natural;
		} else {
			lastWidth = natural;
		}
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

	ContextMenuItem::ContextMenuItem(UIContext &ui, float scale, UString text, std::function<void()> on_select):
		Label(ui, scale),
		onSelect(std::move(on_select)) {
			setText(std::move(text));
			setOnClick([this](Widget &, int button, int, int) {
				if (button != LEFT_BUTTON)
					return false;
				onSelect();
				this->ui.setContextMenu(nullptr); // TODO: is this safe?
				return true;
			});
		}
}
