#include <iostream>

#include "Position.h"
#include "Tiles.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "item/Bomb.h"
#include "realm/Realm.h"
#include "tileentity/Tree.h"

namespace Game3 {
	bool Bomb::use(Slot slot, ItemStack &stack, const Place &place) {
		constexpr Index RADIUS = 4;
		constexpr double CHAR_CHANCE = 0.314159265358979323;

		auto &player = *place.player;
		auto &realm  = *place.realm;
		auto &rng = realm.getGame().dynamicRNG;

		const auto [prow, pcol] = place.position;

		std::uniform_real_distribution one(0., 1.);

		for (Index row = prow - RADIUS + 1; row <= prow + RADIUS - 1; ++row) {
			for (Index column = pcol - RADIUS + 1; column <= pcol + RADIUS - 1; ++column) {
				const Position pos(row, column);
				if (!realm.isValid(pos) || RADIUS < pos.distance(place.position))
					continue;
				if (auto tile = realm.tileEntityAt(pos); tile && tile->getID() == TileEntity::TREE) {
					realm.remove(tile);
					if (one(rng) < CHAR_CHANCE)
						realm.setLayer3(pos, Monomap::CHARRED_STUMP);
					realm.setLayer2(pos, Monomap::ASH);
				}
			}
		}

		realm.getGame().activateContext();
		realm.reupload();

		return true;
	}
}
