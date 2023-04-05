#pragma once

#include "Types.h"

namespace Game3 {
	class Biome {
		public:
			constexpr static BiomeType VOID_ID        = 0;
			constexpr static BiomeType GRASSLAND_ID   = 1;
			constexpr static BiomeType VOLCANIC_ID    = 2;
			constexpr static BiomeType SNOWY_ID       = 3;
			constexpr static BiomeType DESERT_ID      = 4;
			constexpr static BiomeType CAVE_ID        = 5;
			constexpr static BiomeType COUNT          = CAVE_ID + 1;
	};
}