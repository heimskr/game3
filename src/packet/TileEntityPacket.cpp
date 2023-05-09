#include "game/Game.h"
#include "net/Buffer.h"
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
		auto realm = game.realms.at(realmID);
		if (auto iter = realm->tileEntitiesByGID.find(globalID); iter != realm->tileEntitiesByGID.end()) {
			(tileEntity = iter->second)->decode(game, buffer);
		} else {
			auto factory = game.registry<TileEntityFactoryRegistry>()[identifier];
			tileEntity = (*factory)(game);
			tileEntity->globalID = globalID;
			tileEntity->tileEntityID = identifier; // TODO: is this redundant?
			realm->add(tileEntity);
			tileEntity->decode(game, buffer);
		}
	}

	void TileEntityPacket::encode(Game &game, Buffer &buffer) {
		assert(tileEntity);
		buffer << globalID << identifier << realmID;
		tileEntity->encode(game, buffer);
	}
}
