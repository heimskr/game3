#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/ServerGame.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/SetItemFiltersPacket.h"
#include "tileentity/Pipe.h"

namespace Game3 {
	void SetItemFiltersPacket::handle(ServerGame &game, RemoteClient &client) {
		if (!validateDirection(direction)) {
			client.send(ErrorPacket("Can't set item filters: invalid direction"));
			return;
		}

		auto pipe = game.getAgent<Pipe>(pipeGID);
		if (!pipe) {
			client.send(ErrorPacket("Can't set item filters for pipe " + std::to_string(pipeGID) + ": pipe not found"));
			return;
		}

		if (pipe->itemFilters[direction])
			*pipe->itemFilters[direction] = std::move(itemFilter);
		else
			pipe->itemFilters[direction] = std::make_shared<ItemFilter>(std::move(itemFilter));

		pipe->queueBroadcast();
	}
}
