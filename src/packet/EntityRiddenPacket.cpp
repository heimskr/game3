#include "util/Log.h"
#include "entity/Entity.h"
#include "game/ClientGame.h"
#include "packet/EntityRiddenPacket.h"

namespace Game3 {
	EntityRiddenPacket::EntityRiddenPacket(const EntityPtr &rider, const Entity &ridden):
		EntityRiddenPacket(rider? std::make_optional(rider->getGID()) : std::nullopt, ridden.getGID()) {}

	void EntityRiddenPacket::handle(const ClientGamePtr &game) {
		EntityPtr ridden = game->getAgent<Entity>(riddenID);
		if (!ridden) {
			WARN("Couldn't find ridden entity with GID {}", riddenID);
			return;
		}

		if (!riderID) {
			ridden->setRider(nullptr);
			return;
		}

		EntityPtr rider = game->getAgent<Entity>(*riderID);
		if (!rider) {
			WARN("Couldn't find rider entity with GID {}", *riderID);
			return;
		}

		ridden->setRider(rider);
	}
}
