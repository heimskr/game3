#pragma once

#include "ui/gl/Widget.h"

#include <memory>

namespace Game3 {
	class ItemStack;
	class ItemTexture;

	class ItemSlotWidget: public Widget {
		private:
			std::shared_ptr<ItemStack> stack;
			Slot slot = -1;
			double size{};
			double scale{};
			bool active = false;
			std::shared_ptr<ItemTexture> texture;

		public:
			ItemSlotWidget(std::shared_ptr<ItemStack>, Slot, double size, double scale, bool active = false);

			void render(UIContext &, RendererContext &, float x, float y) final;
			std::shared_ptr<Widget> getDragStartWidget() override;
			void setStack(std::shared_ptr<ItemStack>);
			void setActive(bool);
			Slot getSlot() const;
	};
}
