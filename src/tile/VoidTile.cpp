#include "Log.h"
#include "types/Position.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "registry/Registries.h"
#include "tile/VoidTile.h"

#include <boost/json.hpp>

namespace Game3 {
	VoidTile::VoidTile():
		Tile(ID()) {}

	bool VoidTile::interact(const Place &place, Layer layer, const ItemStackPtr &held_item, Hand) {
		if (layer == Layer::Highest && held_item && held_item->hasAttribute("base:attribute/pickaxe"))
			place.set(layer, 0);

		// Returning false even if the tile was mined to let the mineable tile below also get mined.
		return false;
	}
}
