#include "Log.h"
#include "game/ClientGame.h"
#include "packet/DestroyTileEntityPacket.h"

namespace Game3 {
	DestroyTileEntityPacket::DestroyTileEntityPacket(const TileEntity &tile_entity):
		DestroyTileEntityPacket(tile_entity.getGID()) {}

	void DestroyTileEntityPacket::handle(const ClientGamePtr &game) {
		if (auto tile_entity = game->getAgent<TileEntity>(globalID))
			tile_entity->destroy();
		else
			WARN_("DestroyTileEntityPacket: couldn't find tile entity " << globalID << '.');
	}
}
