#include "game/ClientGame.h"
#include "packet/ErrorPacket.h"
#include "ui/MainWindow.h"

namespace Game3 {
	void ErrorPacket::handle(const ClientGamePtr &game) {
		game->getWindow().error(error);
	}
}
