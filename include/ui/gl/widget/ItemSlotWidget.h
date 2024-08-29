#pragma once

#include "ui/gl/widget/Widget.h"

#include <memory>

namespace Game3 {
	class Inventory;
	class ItemStack;
	class ItemTexture;

	class ItemSlotWidget: public Widget {
		private:
			std::shared_ptr<Inventory> inventory;
			std::shared_ptr<ItemStack> stack;
			Slot slot = -1;
			double size{};
			double scale{};
			bool active = false;
			std::shared_ptr<ItemTexture> texture;

		public:
			ItemSlotWidget(std::shared_ptr<Inventory>, std::shared_ptr<ItemStack>, Slot, double size, double scale, bool active = false);
			ItemSlotWidget(Slot, double size, double scale, bool active = false);

			void render(UIContext &, RendererContext &, float x, float y, float width, float height) final;
			std::shared_ptr<Widget> getDragStartWidget() final;
			bool click(UIContext &, int button, int x, int y) final;
			float calculateHeight(RendererContext &, float available_width, float available_height) final;

			void setStack(std::shared_ptr<ItemStack>);
			void setActive(bool);
			Slot getSlot() const;
			std::shared_ptr<Inventory> getInventory() const;
			void setInventory(std::shared_ptr<Inventory>);
			GlobalID getOwnerGID() const;
	};
}
