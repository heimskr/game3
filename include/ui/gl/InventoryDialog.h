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

		public:
			InventoryDialog(std::shared_ptr<Player>);

			void render(UIContext &, RendererContext &) final;
	};
}
