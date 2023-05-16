#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/EntityPacket.h"
#include "entity/Entity.h"
#include "entity/EntityFactory.h"

namespace Game3 {
	EntityPacket::EntityPacket(EntityPtr entity):
		entity(std::move(entity)),
		globalID(entity->globalID),
		realmID(entity->realmID) {}

	void EntityPacket::decode(Game &game, Buffer &buffer) {
		buffer >> globalID >> identifier >> realmID;
		auto realm = game.realms.at(realmID);
		// if (auto iter = realm->entitiesByGID.find(globalID); iter != realm->entitiesByGID.end()) {
		// 	(entity = iter->second)->decode(game, buffer);
		// } else {
		// 	auto factory = game.registry<EntityFactoryRegistry>()[identifier];
		// 	entity = (*factory)(game);
		// 	entity->globalID = globalID;
		// 	entity->entityID = identifier; // TODO: is this redundant?
		// 	realm->add(entity);
		// 	entity->decode(game, buffer);
		// }
	}

	void EntityPacket::encode(Game &game, Buffer &buffer) const {
		assert(entity);
		buffer << globalID << identifier << realmID;
		// entity->encode(game, buffer);
	}
}
