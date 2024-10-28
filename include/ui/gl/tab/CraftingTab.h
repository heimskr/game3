#pragma once

#include "ui/gl/tab/Tab.h"
#include "ui/gl/widget/Box.h"

#include <memory>

namespace Game3 {
	class CraftingTab;
	class InventoryModule;

	class RecipeRow: public Box {
		public:
			RecipeRow(UIContext &, float scale, CraftingRecipePtr);

			void init() final;

		private:
			CraftingRecipePtr recipe;
	};

	class CraftingTab: public Tab {
		public:
			using Tab::Tab;

			void init() final;
			void render(const RendererContext &, float x, float y, float width, float height) final;
			void renderIcon(const RendererContext &) final;
			void onFocus() final;

			void reset();

			/** If the tab is visible, this resets it right away. Otherwise, it queues a reset for the next time the tab is opened. */
			void queueReset();

		private:
			bool resetQueued = false;
			std::shared_ptr<Box> hbox;
			std::shared_ptr<InventoryModule> inventoryModule;
			std::shared_ptr<Box> recipeList;
	};
}
