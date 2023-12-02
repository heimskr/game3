#pragma once

#include "Layer.h"
#include "registry/Registerable.h"

namespace Game3 {
	class ItemStack;
	struct Place;

	class Tile: public NamedRegisterable {
		public:
			Tile(Identifier);
			virtual ~Tile() = default;

			virtual void randomTick(const Place &) {}
			/** Returns false to continue propagation to lower layers, true to stop it. */
			virtual bool interact(const Place &, Layer, ItemStack *used_item);
	};
}
