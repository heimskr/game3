#include "Log.h"
#include "game/ClientGame.h"
#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/EntityPacket.h"
#include "entity/Entity.h"
#include "entity/EntityFactory.h"

namespace Game3 {
	EntityPacket::EntityPacket(EntityPtr entity_):
		entity(std::move(entity_)),
		identifier(entity->type),
		globalID(entity->globalID),
		realmID(entity->realmID) {}

	void EntityPacket::decode(Game &game, Buffer &buffer) {
		buffer >> globalID >> identifier >> realmID;
		auto realm = game.realms.at(realmID);
		if (auto iter = realm->entitiesByGID.find(globalID); iter != realm->entitiesByGID.end()) {
			if (identifier == "base:entity/player"_id)
				SUCCESS("Found player " << globalID);
			(entity = iter->second)->decode(buffer);
		} else {
			if (identifier == "base:entity/player"_id)
				ERROR("Didn't find player " << globalID);
			auto factory = game.registry<EntityFactoryRegistry>()[identifier];
			entity = (*factory)(game, {});
			entity->globalID = globalID;
			entity->type = identifier;
			entity->init(game);
			entity->decode(buffer);
		}
	}

	void EntityPacket::encode(Game &, Buffer &buffer) const {
		assert(entity);
		buffer << globalID << identifier << realmID;
		entity->encode(buffer);
	}

	void EntityPacket::handle(ClientGame &game) {
		game.realms.at(realmID)->add(entity);
	}
}
