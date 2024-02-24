#include <iostream>

#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "item/Bomb.h"
#include "realm/Realm.h"

namespace Game3 {
	bool Bomb::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<double, double>) {
		constexpr Index  DIAMETER = 5;
		constexpr double RADIUS = DIAMETER / 2.;

		auto &realm  = *place.realm;
		const auto [prow, pcol] = place.position;

		for (Index row = prow - DIAMETER; row <= prow + DIAMETER; ++row) {
			for (Index column = pcol - DIAMETER; column <= pcol + DIAMETER; ++column) {
				const Position pos(row, column);
				if (RADIUS < pos.distance(place.position))
					continue;
				realm.damageGround(pos);
				if (auto tile = realm.tileEntityAt(pos); tile && tile->kill())
					tile->destroy();
			}
		}

		const InventoryPtr inventory = place.player->getInventory(0);
		auto inventory_lock = inventory->uniqueLock();
		if (--stack.count == 0)
			inventory->erase(slot);
		inventory->notifyOwner();

		return true;
	}
}
