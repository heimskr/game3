#include "Log.h"
#include "error/PlayerMissingError.h"
#include "game/ClientGame.h"
#include "packet/FluidUpdatePacket.h"

namespace Game3 {
	void FluidUpdatePacket::handle(const ClientGamePtr &game) {
		auto realm = game->getRealm(realmID);
		realm->setFluid(position, fluidTile);
		realm->queueReupload();
	}
}
