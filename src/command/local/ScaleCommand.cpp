#include "command/local/ScaleCommand.h"
#include "game/ClientGame.h"
#include "net/LocalClient.h"
#include "ui/Window.h"
#include "util/Util.h"

namespace Game3 {
	void ScaleCommand::operator()(LocalClient &client) {
		if (pieces.size() < 2) {
			throw CommandError("\"scale\" command requires 1 arguments: map scale");
		}

		try {
			client.getGame()->getWindow()->scale = parseNumber<double>(pieces[1]);
		} catch (const std::invalid_argument &) {
			throw CommandError("Invalid scale.");
		}
	}
}
