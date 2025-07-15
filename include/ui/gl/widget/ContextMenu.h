#pragma once

#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/HasFixedWidth.h"

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

		private:
			std::vector<std::shared_ptr<ContextMenuItem>> items;
			WidgetPtr anchor;
			float xOffset{};
			float yOffset{};
			float lastWidth{};
			float lastHeight{};
	};
}
