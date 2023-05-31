#include "game/ClientGame.h"
#include "packet/TimePacket.h"

namespace Game3 {
	void TimePacket::handle(ClientGame &game) {
		game.time = time;
	}
}
