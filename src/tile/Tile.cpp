#include "tile/Tile.h"

namespace Game3 {
	Tile::Tile(Identifier identifier_):
		NamedRegisterable(std::move(identifier_)) {}

	bool Tile::interact(const Place &, Layer, ItemStack *) {
		return false;
	}
}
