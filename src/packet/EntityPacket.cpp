#include "util/Log.h"
#include "game/ClientGame.h"
#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/EntityPacket.h"
#include "packet/PacketError.h"
#include "entity/Entity.h"
#include "entity/EntityFactory.h"

namespace Game3 {
	Lockable<std::unordered_map<std::string, size_t>> entityUpdates;

	EntityPacket::EntityPacket(EntityPtr entity_):
		entity(std::move(entity_)),
		identifier(entity->type),
		globalID(entity->globalID),
		realmID(entity->realmID) {
			storedBuffer.context = entity->getGame()->weak_from_this();
			entity->encode(storedBuffer);
		}

	void EntityPacket::decode(Game &game, Buffer &buffer) {
		buffer >> globalID >> identifier >> realmID;
		assert(globalID != static_cast<GlobalID>(-1));
		assert(globalID != static_cast<GlobalID>(0));
		storedBuffer = std::move(buffer);
		storedBuffer.context = game.weak_from_this();
		buffer.context = storedBuffer.context;
	}

	void EntityPacket::encode(Game &, Buffer &buffer) const {
		assert(entity);
		buffer << globalID << identifier << realmID << storedBuffer;
	}

	void EntityPacket::handle(const ClientGamePtr &game) {
		RealmPtr realm = game->tryRealm(realmID);

		if (!realm)
			throw PacketError("Couldn't find realm " + std::to_string(realmID) + " in EntityPacket");

		if (EntityPtr found = game->getAgent<Entity>(globalID)) {
			wasFound = true;
			(entity = found)->decode(storedBuffer);
		} else {
			{ auto lock = game->allAgents.sharedLock(); assert(!game->allAgents.contains(globalID)); }
			wasFound = false;
			auto factory = game->registry<EntityFactoryRegistry>()[identifier];
			entity = (*factory)(game);
			entity->type = identifier;
			entity->setGID(globalID);
			entity->init(game);
			entity->decode(storedBuffer);
		}

		{
			auto lock = entityUpdates.uniqueLock();
			++entityUpdates[entity->getName()];
		}

		if (wasFound)
			return;

		realm->add(entity, entity->getPosition());
	}
}
