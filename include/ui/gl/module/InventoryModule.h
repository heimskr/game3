#pragma once

#include "graphics/Rectangle.h"
#include "types/Types.h"
#include "ui/gl/module/Module.h"
#include "ui/gl/widget/ItemSlotWidget.h"

namespace Game3 {
	class InventoryModule: public Module {
		private:
			Rectangle innerRectangle;
			std::vector<std::shared_ptr<ItemSlotWidget>> slotWidgets;
			Slot previousActive = -1;

		public:
			using Module::Module;

			Identifier getID() const final { return {"base", "module/inventory"}; }

			void render(UIContext &, RendererContext &, float x, float y, float width, float height) final;
			bool click(UIContext &, int x, int y) final;
			bool dragStart(UIContext &, int x, int y) final;
			bool dragEnd(UIContext &, int x, int y) final;
	};
}
