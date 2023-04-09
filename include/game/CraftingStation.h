#pragma once

#include <cstddef>

namespace Game3 {
	struct Agent;

	struct CraftingStationBase {
		virtual ~CraftingStationBase() = 0;
	};

	template <typename T>
	class CraftingStation: public CraftingStationBase {
		public:
			virtual bool craft(Agent &, size_t quantity);
	};
}
