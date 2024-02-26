#include "game/ClientGame.h"
#include "packet/TimePacket.h"

namespace Game3 {
	void TimePacket::handle(const ClientGamePtr &game) {
		game->time = time;
	}
}
