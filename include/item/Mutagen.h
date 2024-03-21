#pragma once

#include "item/FilledFlask.h"

namespace Game3 {
	class Mutagen: public FilledFlask {
		public:
			using FilledFlask::FilledFlask;
			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
	};
}
