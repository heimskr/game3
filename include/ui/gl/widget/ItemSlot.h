#pragma once

#include "ui/gl/widget/Widget.h"

#include <memory>

namespace Game3 {
	class Inventory;
	class ItemStack;
	class ItemTexture;

	class ItemSlot: public Widget {
		private:
			std::shared_ptr<Inventory> inventory;
			std::shared_ptr<ItemStack> stack;
			Slot slot = -1;
			float size{};
			bool active = false;
			std::shared_ptr<ItemTexture> texture;

		public:
			ItemSlot(UIContext &, std::shared_ptr<Inventory>, std::shared_ptr<ItemStack>, Slot, float size, float scale, bool active = false);
			ItemSlot(UIContext &, Slot, float size, float scale, bool active = false);

			using Widget::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;

			std::shared_ptr<Widget> getDragStartWidget() final;
			bool click(int button, int x, int y) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			void setStack(std::shared_ptr<ItemStack>);
			void setActive(bool);
			Slot getSlot() const;
			std::shared_ptr<Inventory> getInventory() const;
			void setInventory(std::shared_ptr<Inventory>);
			GlobalID getOwnerGID() const;
	};
}
