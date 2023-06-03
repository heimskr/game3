#include "Log.h"
#include "error/PlayerMissingError.h"
#include "game/ClientGame.h"
#include "packet/FluidUpdatePacket.h"

namespace Game3 {
	void FluidUpdatePacket::handle(ClientGame &game) {
		auto realm = game.realms.at(realmID);
		realm->setFluid(position, fluidTile, false);
		realm->reuploadFluids();
	}
}