#include "Log.h"
#include "game/ClientGame.h"
#include "packet/DestroyTileEntityPacket.h"

namespace Game3 {
	DestroyTileEntityPacket::DestroyTileEntityPacket(const TileEntity &tile_entity):
		DestroyTileEntityPacket(tile_entity.getGID(), tile_entity.realmID) {}

	void DestroyTileEntityPacket::handle(ClientGame &game) {
		auto realm_iter = game.realms.find(realmID);

		if (realm_iter == game.realms.end()) {
			WARN("DestroyTileEntityPacket: couldn't find realm " << realmID);
			return;
		}

		auto realm = realm_iter->second;
		auto tile_entity = realm->getTileEntity(globalID);

		if (!tile_entity) {
			WARN("DestroyTileEntityPacket: couldn't find tile entity " << globalID << " in realm " << realmID);
			return;
		}

		tile_entity->destroy();
	}
}
