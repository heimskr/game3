#include "Log.h"
#include "game/ClientGame.h"
#include "entity/ClientPlayer.h"
#include "packet/SetPlayerStationTypesPacket.h"

namespace Game3 {
	void SetPlayerStationTypesPacket::handle(ClientGame &game) {
		game.player->stationTypes = std::move(stationTypes);
	}
}
