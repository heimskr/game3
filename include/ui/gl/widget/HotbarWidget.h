#pragma once

#include "types/Types.h"
#include "ui/gl/widget/ItemSlotWidget.h"
#include "ui/gl/widget/Widget.h"

#include <vector>

namespace Game3 {
	class HotbarWidget: public Widget {
		private:
			Slot slotCount{};
			double scale{};
			std::vector<ItemSlotWidget> slotWidgets;

		public:
			HotbarWidget(Slot slot_count, double scale);

			void render(UIContext &, RendererContext &, float x, float y, float width, float height) final;
			bool click(UIContext &, int x, int y) final;
	};
}
