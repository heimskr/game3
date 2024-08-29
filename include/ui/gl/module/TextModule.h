#pragma once

#include "game/InventoryGetter.h"
#include "graphics/Rectangle.h"
#include "types/Types.h"
#include "ui/gl/module/Module.h"
#include "ui/gl/widget/ItemSlotWidget.h"

#include <any>

namespace Game3 {
	class ClientGame;
	class ClientInventory;

	class InventoryModule: public Module {
		public:
			struct Argument {
				std::shared_ptr<Agent> agent;
				InventoryID index;
			};

			InventoryModule(std::shared_ptr<ClientGame>, const std::any &);
			InventoryModule(std::shared_ptr<ClientGame>, const std::string &);

			static Identifier ID() { return {"base", "module/inventory"}; }

			Identifier getID() const final { return ID(); }

			void render(UIContext &, RendererContext &, float x, float y, float width, float height) final;
			bool click(UIContext &, int x, int y) final;
			bool dragStart(UIContext &, int x, int y) final;
			bool dragEnd(UIContext &, int x, int y) final;

		private:
			Rectangle innerRectangle;
			std::vector<std::shared_ptr<ItemSlotWidget>> slotWidgets;
			std::unique_ptr<InventoryGetter> inventoryGetter;
			Slot previousActive = -1;

			static std::shared_ptr<ClientInventory> getInventory(const std::any &);
	};
}
