#pragma once

#include "ui/gl/Widget.h"

#include <memory>

namespace Game3 {
	class ItemStack;
	class ItemTexture;

	class ItemSlotWidget: public Widget {
		private:
			std::shared_ptr<ItemStack> stack;
			double size{};
			double scale{};
			bool active = false;
			std::shared_ptr<ItemTexture> texture;

		public:
			ItemSlotWidget(std::shared_ptr<ItemStack>, double size, double scale, bool active = false);

			void render(UIContext &, RendererContext &, float x, float y) final;
			void setStack(std::shared_ptr<ItemStack>);
			void setActive(bool);
	};
}
