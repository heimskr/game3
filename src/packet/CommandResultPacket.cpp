#include "game/ClientGame.h"
#include "packet/CommandResultPacket.h"
#include "ui/Window.h"

namespace Game3 {
	void CommandResultPacket::handle(const ClientGamePtr &game) {
		auto window = game->getWindow();
		if (message.empty())
			return;
		if (success)
			window->alert("Command " + std::to_string(commandID) + " was successful:\n\n" + message);
		else
			window->error("Command " + std::to_string(commandID) + " was unsuccessful:\n\n" + message);
	}
}
