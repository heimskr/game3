#pragma once

#include "game/InventoryGetter.h"
#include "types/Types.h"
#include "ui/gl/module/Module.h"
#include "ui/gl/widget/ItemSlot.h"

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

			InventoryModule(UIContext &, const std::shared_ptr<ClientGame> &, const std::any &);
			InventoryModule(UIContext &, const std::shared_ptr<ClientInventory> &);

			static Identifier ID() { return {"base", "module/inventory"}; }

			Identifier getID() const final { return ID(); }
			void init() final;

			using Module::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;

			bool click(int button, int x, int y) final;
			bool dragStart(int x, int y) final;
			bool dragEnd(int x, int y) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			void setTopPadding(float);
			float getTopPadding() const;

		private:
			std::vector<std::shared_ptr<ItemSlot>> slotWidgets;
			std::unique_ptr<InventoryGetter> inventoryGetter;
			Slot previousActive = -1;
			float topPadding = 0;
	};
}
