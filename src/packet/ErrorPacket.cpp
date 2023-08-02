#include "game/ClientGame.h"
#include "packet/ErrorPacket.h"
#include "ui/MainWindow.h"

namespace Game3 {
	void ErrorPacket::handle(ClientGame &game) {
		game.getWindow().error(error);
	}
}
