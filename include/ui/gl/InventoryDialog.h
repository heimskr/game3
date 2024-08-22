#pragma once

#include <memory>

#include "ui/gl/Dialog.h"
#include "ui/gl/ItemSlotWidget.h"

namespace Game3 {
	class Player;

	class InventoryDialog: public Dialog {
		private:
			std::shared_ptr<Player> player;
			std::vector<std::shared_ptr<ItemSlotWidget>> slotWidgets;
			Slot previousActive = -1;
			Rectangle innerRectangle;

		public:
			InventoryDialog(UIContext &, std::shared_ptr<Player>);

			void render(RendererContext &) final;
			Rectangle getPosition() const final;
			bool dragStart(int x, int y) final;
			bool dragEnd(int x, int y) final;
	};
}
