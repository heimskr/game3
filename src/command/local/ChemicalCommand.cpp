#include "command/local/ChemicalCommand.h"
#include "game/ClientGame.h"
#include "net/LocalClient.h"

namespace Game3 {
	void ChemicalCommand::operator()(LocalClient &client) {
		if (pieces.size() != 2 && pieces.size() != 3)
			throw CommandError("\"chem\" command arguments: <formula> [count]");
		std::string count = "1";
		if (pieces.size() == 3)
			count = pieces[2];
		client.getGame()->runCommand("give chemical " + count + " " + boost::json::serialize(boost::json::value{{"formula", std::move(pieces.at(1))}}));
	}
}
