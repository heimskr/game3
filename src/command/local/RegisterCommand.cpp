#include "command/local/RegisterCommand.h"
#include "net/LocalClient.h"
#include "packet/RegisterPlayerPacket.h"
#include "util/Util.h"

namespace Game3 {
	void RegisterCommand::operator()(LocalClient &client) {
		if (pieces.size() < 3)
			throw CommandError("\"register\" command requires 2 arguments: username, display name");
		client.send(RegisterPlayerPacket(pieces.at(1), join(std::span(pieces).subspan(2), " ")));
	}
}
