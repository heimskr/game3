#pragma once

#include "graphics/Rectangle.h"
#include "types/Types.h"
#include "ui/gl/widget/ItemSlot.h"
#include "ui/gl/widget/Widget.h"

#include <vector>

namespace Game3 {
	class Hotbar: public Widget {
		public:
			Hotbar(float scale);

			using Widget::render;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;

			bool click(UIContext &, int button, int x, int y) final;
			bool dragStart(UIContext &, int x, int y) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

		private:
			std::vector<std::shared_ptr<ItemSlot>> slotWidgets;
	};
}
