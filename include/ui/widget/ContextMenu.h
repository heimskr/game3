#pragma once

#include "ui/widget/Box.h"
#include "ui/widget/Label.h"
#include "ui/HasFixedWidth.h"

namespace Game3 {
	class ContextMenuItem: public Label {
		public:
			std::function<void()> onSelect;

			ContextMenuItem(UIContext &, float scale, UString text, std::function<void()> onSelect = {});
	};

	class ContextMenu: public Box, public HasFixedWidth {
		public:
			ContextMenu(UIContext &, float scale, WidgetPtr anchor, float x_offset, float y_offset);

			using Widget::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;
			bool blocksMouse(int x, int y, bool is_drag_update) const final;

			bool keyPressed(uint32_t key, Modifiers, bool is_repeat) final;

			void addItem(std::shared_ptr<ContextMenuItem>);

			WidgetPtr getAnchor() const;

			/** If the bottom edge and/or right edge of this context menu extends past that of the game UI, this moves the offset up and/or left. */
			void adjustPosition();

			float getXOffset() const;
			float getYOffset() const;

			void setXOffset(float);
			void setYOffset(float);

		private:
			std::vector<std::shared_ptr<ContextMenuItem>> items;
			WidgetPtr anchor;
			float xOffset{};
			float yOffset{};
			float lastWidth{};
			float lastHeight{};
	};
}
