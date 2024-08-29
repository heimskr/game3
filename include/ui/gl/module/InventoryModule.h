#pragma once

#include "game/InventoryGetter.h"
#include "graphics/Rectangle.h"
#include "types/Types.h"
#include "ui/gl/module/Module.h"
#include "ui/gl/widget/ItemSlotWidget.h"

#include <any>
#include <memory>
#include <vector>

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
			InventoryModule(std::shared_ptr<ClientGame>, const std::shared_ptr<ClientInventory> &);

			static Identifier ID() { return {"base", "module/inventory"}; }

			Identifier getID() const final { return ID(); }

			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			bool click(UIContext &, int button, int x, int y) final;
			bool dragStart(UIContext &, int x, int y) final;
			bool dragEnd(UIContext &, int x, int y) final;
			float calculateHeight(const RendererContext &, float available_width, float available_height) final;

		private:
			std::vector<std::shared_ptr<ItemSlotWidget>> slotWidgets;
			std::unique_ptr<InventoryGetter> inventoryGetter;
			Slot previousActive = -1;

			static std::shared_ptr<ClientInventory> getInventory(const std::any &);
	};
}
