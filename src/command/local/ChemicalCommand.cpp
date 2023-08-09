#include "command/local/ChemicalCommand.h"
#include "game/ClientGame.h"
#include "net/LocalClient.h"

namespace Game3 {
	void ChemicalCommand::operator()(LocalClient &client) {
		if (pieces.size() != 2)
			throw CommandError("\"chem\" command requires 1 argument: formula");
		client.lockGame()->runCommand("give chemical 1 " + nlohmann::json{{"formula", std::move(pieces.at(1))}}.dump());
	}
}
