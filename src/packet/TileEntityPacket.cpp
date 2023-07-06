#include "Log.h"
#include "game/ClientGame.h"
#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/PacketError.h"
#include "packet/TileEntityPacket.h"
#include "tileentity/TileEntity.h"
#include "tileentity/TileEntityFactory.h"

namespace Game3 {
	TileEntityPacket::TileEntityPacket(TileEntityPtr tile_entity):
		tileEntity(std::move(tile_entity)),
		identifier(tileEntity->getID()),
		globalID(tileEntity->globalID),
		realmID(tileEntity->realmID) {}

	void TileEntityPacket::decode(Game &game, Buffer &buffer) {
		buffer >> globalID >> identifier >> realmID;
		assert(globalID != static_cast<GlobalID>(-1));
		assert(globalID != static_cast<GlobalID>(0));

		auto realm_iter = game.realms.find(realmID);
		if (realm_iter == game.realms.end())
			throw PacketError("Couldn't find realm " + std::to_string(realmID) + " in TileEntityPacket");

		auto realm = realm_iter->second;

		if (auto found = game.getAgent<TileEntity>(globalID)) {
			wasFound = true;
			(tileEntity = found)->decode(game, buffer);
		} else {
			{ auto lock = game.allAgents.sharedLock(); assert(!game.allAgents.contains(globalID)); }
			wasFound = false;
			auto factory = game.registry<TileEntityFactoryRegistry>()[identifier];
			tileEntity = (*factory)(game);
			tileEntity->globalID = globalID;
			tileEntity->tileEntityID = identifier;
			tileEntity->init(game);
			tileEntity->decode(game, buffer);
			realm->add(tileEntity);
		}
	}

	void TileEntityPacket::encode(Game &game, Buffer &buffer) const {
		assert(tileEntity);
		buffer << globalID << identifier << realmID;
		tileEntity->encode(game, buffer);
	}

	void TileEntityPacket::handle(ClientGame &game) {
		if (tileEntity && !wasFound)
			game.realms.at(realmID)->add(tileEntity);
	}
}
