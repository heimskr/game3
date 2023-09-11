#include "game/ClientGame.h"
#include "packet/ErrorPacket.h"
#include "ui/MainWindow.h"

namespace Game3 {
	void ErrorPacket::handle(ClientGame &game) {
		// TODO!: nanogui
		// game.getWindow().error(error);
	}
}
