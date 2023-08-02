#include "game/ClientGame.h"
#include "packet/CommandResultPacket.h"
#include "ui/MainWindow.h"

namespace Game3 {
	void CommandResultPacket::handle(ClientGame &game) {
		if (success)
			game.getWindow().alert("Command " + std::to_string(commandID) + " was successful:\n\n" + message);
		else
			game.getWindow().error("Command " + std::to_string(commandID) + " was unsuccessful:\n\n" + message);
	}
}
