#pragma once

#include "math/Rectangle.h"
#include "types/Types.h"
#include "ui/widget/ItemSlot.h"
#include "ui/widget/Widget.h"

#include <vector>

namespace Game3 {
	class Hotbar: public Widget {
		public:
			Hotbar(UIContext &, float selfScale);

			void init() final;
			using Widget::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;

			bool blocksMouse(int x, int y, bool is_drag_update) const final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			void reset();

			const std::optional<float> & getLastY() const;

		private:
			std::vector<std::shared_ptr<ItemSlot>> slotWidgets;
			std::shared_ptr<ItemSlot> heldLeft;
			std::shared_ptr<ItemSlot> heldRight;
			std::optional<float> lastY;

			float getRightSideWidth() const;
	};
}
