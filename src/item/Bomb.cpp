#include <iostream>

#include "Position.h"
#include "Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "item/Bomb.h"
#include "realm/Realm.h"
#include "tileentity/Tree.h"

namespace Game3 {
	bool Bomb::use(Slot, ItemStack &, const Place &place, Modifiers) {
		constexpr double RADIUS = 2.5;

		auto &realm  = *place.realm;
		const auto [prow, pcol] = place.position;

		for (Index row = prow - RADIUS * 2; row <= prow + RADIUS * 2; ++row) {
			for (Index column = pcol - RADIUS * 2; column <= pcol + RADIUS * 2; ++column) {
				const Position pos(row, column);
				if (RADIUS < pos.distance(place.position))
					continue;
				realm.damageGround(pos);
				if (auto tile = realm.tileEntityAt(pos); tile && tile->kill())
					realm.remove(tile);
			}
		}

		realm.getGame().activateContext();
		realm.reupload();

		return true;
	}
}
