#pragma once

#include "item/BasicFood.h"

namespace Game3 {
	class Cake: public BasicFood {
		public:
			using BasicFood::BasicFood;

			using BasicFood::use;
			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
	};
}
