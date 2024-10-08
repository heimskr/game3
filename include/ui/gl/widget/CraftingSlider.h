#pragma once

#include "ui/gl/widget/Grid.h"

#include <memory>

namespace Game3 {
	class IntegerInput;
	class Slider;

	class CraftingSlider: public Grid {
		public:
			using Grid::Grid;

			void init() final;

		private:
			std::shared_ptr<IntegerInput> valueInput;
			std::shared_ptr<Slider> valueSlider;
			std::size_t value = 1;

			void setValue(std::size_t);
			void increment(ssize_t delta);
			void craft();
			std::size_t getMaximum() const;
	};
}
