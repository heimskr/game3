#include <iostream>

#include "entity/Player.h"
#include "game/ServerGame.h"
#include "realm/Realm.h"
#include "tileentity/Stockpile.h"

namespace Game3 {
	Stockpile::Stockpile(Identifier tilename, const Position &position_, VillageID village_id):
	TileEntity(std::move(tilename), getID(), position_, true), villageID(village_id) {
		tileEntityID = ID();
	}

	bool Stockpile::onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, ItemStack *, Hand) {
		if (getSide() != Side::Server)
			return false;

		VillagePtr village = getGame().toServer().getVillage(villageID);
		return true;
	}
}
