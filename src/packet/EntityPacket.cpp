#include "Log.h"
#include "game/ClientGame.h"
#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/EntityPacket.h"
#include "packet/PacketError.h"
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
		assert(globalID != static_cast<GlobalID>(-1));
		assert(globalID != static_cast<GlobalID>(0));

		RealmPtr realm = game.tryRealm(realmID);
		if (!realm)
			throw PacketError("Couldn't find realm " + std::to_string(realmID) + " in EntityPacket");

		if (EntityPtr found = game.getAgent<Entity>(globalID)) {
			wasFound = true;
			(entity = found)->decode(buffer);
		} else {
			{ auto lock = game.allAgents.sharedLock(); assert(!game.allAgents.contains(globalID)); }
			wasFound = false;
			auto factory = game.registry<EntityFactoryRegistry>()[identifier];
			entity = (*factory)(game);
			entity->type = identifier;
			entity->setGID(globalID);
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
		if (wasFound)
			return;
		RealmPtr realm = game.tryRealm(realmID);
		if (!realm)
			throw PacketError("Couldn't find realm " + std::to_string(realmID) + " in EntityPacket");
		realm->add(entity, entity->getPosition());
	}
}
