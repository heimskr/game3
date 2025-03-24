#pragma once

#include "ui/gl/widget/Widget.h"
#include "ui/gl/HasAlignment.h"
#include "ui/gl/HasTooltipText.h"

#include <memory>
#include <sigc++/sigc++.h>

namespace Game3 {
	class Inventory;
	class ItemStack;

	class ItemSlot: public Widget, public HasAlignment, public HasTooltipText {
		public:
			sigc::signal<void(ItemSlot &self, const WidgetPtr &dragged)> onDrop;

			ItemSlot(UIContext &, std::shared_ptr<Inventory>, std::shared_ptr<ItemStack>, Slot, float size, float selfScale, bool active = false);
			ItemSlot(UIContext &, Slot, float size, float selfScale, bool active = false);
			ItemSlot(UIContext &, Slot = -1, bool active = false);

			using Widget::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;

			std::shared_ptr<Widget> getDragStartWidget() final;
			bool click(int button, int x, int y, Modifiers) final;
			bool dragStart(int x, int y) final;
			bool dragEnd(int x, int y) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			void setStack(ItemStackPtr);
			ItemStackPtr getStack() const;
			void setActive(bool);
			Slot getSlot() const;
			std::shared_ptr<Inventory> getInventory() const;
			void setInventory(std::shared_ptr<Inventory>);
			GlobalID getOwnerGID() const;

		private:
			std::shared_ptr<Inventory> inventory;
			std::shared_ptr<ItemStack> stack;
			Slot slot = -1;
			float size{};
			bool active = false;
			std::shared_ptr<Texture> texture;
	};
}
