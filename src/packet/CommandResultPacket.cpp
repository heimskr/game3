#include "game/ClientGame.h"
#include "packet/CommandResultPacket.h"
#include "ui/Window.h"

namespace Game3 {
	void CommandResultPacket::handle(const ClientGamePtr &game) {
		WindowPtr window = game->getWindow();

		if (message.empty()) {
			return;
		}

		if (success) {
			game->handleChat(nullptr, message);
		} else {
			game->handleChat(nullptr, std::format("Command {} was unsuccessful: {}", commandID, message));
		}
	}
}
