#pragma once

#include "registry/Registerable.h"

namespace Game3 {
	struct Place;

	class Tile: public NamedRegisterable {
		public:
			Tile(Identifier);
			virtual ~Tile() = default;

			virtual void randomTick(const Place &) {}
	};
}
