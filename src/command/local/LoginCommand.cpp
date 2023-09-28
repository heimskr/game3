#include "command/local/LoginCommand.h"
#include "net/LocalClient.h"
#include "packet/LoginPacket.h"
#include "util/Util.h"

namespace Game3 {
	void LoginCommand::operator()(LocalClient &client) {
		if (pieces.size() != 2)
			throw CommandError("\"login\" command requires 1 arguments: username");

		if (!client.hasHostname())
			throw CommandError("Can't log in: not connected");

		const std::string &username = pieces[1];
		const std::string &hostname = client.getHostname();
		if (std::optional<Token> token = client.getToken(hostname, username))
			client.send(LoginPacket(username, *token));
		else
			throw CommandError("Token for user " + username + " on host " + hostname + " not found; try registering");
	}
}
