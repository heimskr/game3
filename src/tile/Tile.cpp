#include "tile/Tile.h"

namespace Game3 {
	Tile::Tile(Identifier identifier_):
		NamedRegisterable(std::move(identifier_)) {}
}
