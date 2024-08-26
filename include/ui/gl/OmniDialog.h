#pragma once

#include "graphics/Rectangle.h"
#include "threading/LockableSharedPtr.h"
#include "types/Types.h"
#include "ui/gl/Dialog.h"

#include <memory>
#include <vector>

namespace Game3 {
	class CraftingTab;
	class InventoryTab;
	class Tab;

	class OmniDialog: public Dialog {
		public:
			std::shared_ptr<InventoryTab> inventoryTab;
			std::shared_ptr<CraftingTab> craftingTab;
			std::vector<std::shared_ptr<Tab>> tabs;
			std::vector<Rectangle> tabRectangles;
			std::shared_ptr<Tab> activeTab;

			OmniDialog(UIContext &);

			void render(RendererContext &) final;
			Rectangle getPosition() const final;
			bool click(int x, int y) final;
			bool dragStart(int x, int y) final;
			bool dragEnd(int x, int y) final;
	};
}
