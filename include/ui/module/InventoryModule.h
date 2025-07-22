#pragma once

#include "game/InventoryGetter.h"
#include "types/Types.h"
#include "ui/module/Module.h"
#include "ui/widget/ItemSlot.h"

#include <any>
#include <functional>
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

			InventoryModule(UIContext &, float selfScale, const std::shared_ptr<ClientGame> &, const std::any &);
			InventoryModule(UIContext &, float selfScale, const std::shared_ptr<ClientInventory> &);

			static Identifier ID() { return {"base", "module/inventory"}; }

			Identifier getID() const final { return ID(); }
			void init() final;

			using Module::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;

			bool click(int button, int x, int y, Modifiers) final;
			bool dragEnd(int x, int y, double) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			std::shared_ptr<InventoryModule> getPrimaryInventoryModule() final;

			void setTopPadding(float);
			float getTopPadding() const;

			InventoryPtr getInventory() const;
			void setOnSlotClick(std::function<bool(Slot, Modifiers)>);

		private:
			std::vector<std::shared_ptr<ItemSlot>> slotWidgets;
			std::unique_ptr<InventoryGetter> inventoryGetter;
			/** Returns true to override the default click behavior. */
			std::function<bool(Slot, Modifiers)> onSlotClick;
			Slot previousActive = -1;
			float topPadding = 0;

			bool clickSlot(Slot, Modifiers);
	};
}
