#pragma once

#include "graphics/Rectangle.h"
#include "types/Types.h"
#include "ui/gl/widget/ItemSlotWidget.h"
#include "ui/gl/widget/Widget.h"

#include <vector>

namespace Game3 {
	class HotbarWidget: public Widget {
		private:
			float scale{};
			std::vector<std::shared_ptr<ItemSlotWidget>> slotWidgets;

		public:
			HotbarWidget(float scale);

			void render(UIContext &, RendererContext &, float x, float y, float width, float height) final;
			bool click(UIContext &, int button, int x, int y) final;
			bool dragStart(UIContext &, int x, int y) final;
			float calculateHeight(RendererContext &, float available_width, float available_height) final;
	};
}
