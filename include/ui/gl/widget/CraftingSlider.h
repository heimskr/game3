#pragma once

#include "ui/gl/widget/Grid.h"

#include <memory>

namespace Game3 {
	class IntegerInput;
	class Slider;
	struct CraftingRecipe;

	class CraftingSlider: public Grid {
		public:
			CraftingSlider(UIContext &, float scale, std::shared_ptr<CraftingRecipe> recipe);

			void init() final;

		private:
			std::shared_ptr<CraftingRecipe> recipe;
			std::shared_ptr<IntegerInput> valueInput;
			std::shared_ptr<Slider> valueSlider;
			std::size_t value = 1;

			void setValue(std::size_t);
			void increment(ssize_t delta);
			void craft();
			std::size_t getMaximum() const;
	};
}
