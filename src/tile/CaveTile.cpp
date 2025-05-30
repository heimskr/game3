#include "util/Log.h"
#include "lib/JSON.h"
#include "realm/Cave.h"
#include "tile/CaveTile.h"
#include "types/Position.h"

namespace Game3 {
	CaveTile::CaveTile(Identifier identifier, ItemStackPtr stack, Identifier floor):
		MineableTile(std::move(identifier), std::move(stack), true),
		floor(std::move(floor)) {}

	bool CaveTile::interact(const Place &place, Layer layer, const ItemStackPtr &used_item, Hand hand) {
		if (!MineableTile::interact(place, layer, used_item, hand)) {
			return false;
		}

		reveal(place);
		return true;
	}

	bool CaveTile::damage(const Place &place, Layer layer) {
		if (!MineableTile::damage(place, layer)) {
			return false;
		}

		reveal(place);
		return true;
	}

	void CaveTile::reveal(const Place &place) const {
		if (std::optional<TileID> terrain = place.get(Layer::Bedrock); !terrain || *terrain == 0) {
			place.set(Layer::Bedrock, floor);
		}

		if (auto cave = std::dynamic_pointer_cast<Cave>(place.realm)) {
			cave->reveal(place.position, true);
		}
	}
}
