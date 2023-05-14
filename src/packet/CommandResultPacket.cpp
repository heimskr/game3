#include "Log.h"
#include "game/ClientGame.h"
#include "game/TileProvider.h"
#include "packet/CommandResultPacket.h"
#include "packet/PacketError.h"

namespace Game3 {
	void CommandResultPacket::handle(ClientGame &) {
		INFO("Command " << commandID << " was " << (success? "" : "un") << "successful: " << message);
	}
}
