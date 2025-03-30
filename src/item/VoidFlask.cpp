#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "graphics/Tileset.h"
#include "item/VoidFlask.h"
#include "realm/Realm.h"
#include "tileentity/FluidHoldingTileEntity.h"
#include "types/Position.h"

namespace Game3 {
	bool VoidFlask::use(Slot, const ItemStackPtr &, const Place &place, Modifiers, std::pair<float, float>) {
		RealmPtr realm = place.realm;
		assert(realm->getSide() == Side::Server);

		if (std::optional<FluidTile> tile = place.getFluid(); tile && 0 < tile->level) {
			tile->level = 0;
			realm->setFluid(place.position, *tile);
			return true;
		}

		if (auto tile_entity = std::dynamic_pointer_cast<FluidHoldingTileEntity>(place.getTileEntity())) {
			tile_entity->setFluidLevels({});
			return true;
		}

		return false;
	}

	bool VoidFlask::drag(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets) {
		if (modifiers.onlyShift()) {
			return use(slot, stack, place, modifiers, offsets);
		}
		return false;
	}
}
