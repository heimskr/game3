#include "Log.h"
#include "game/ClientGame.h"
#include "packet/DestroyEntityPacket.h"

namespace Game3 {
	DestroyEntityPacket::DestroyEntityPacket(const Entity &entity):
		DestroyEntityPacket(entity.getGID()) {}

	void DestroyEntityPacket::handle(ClientGame &game) {
		if (auto entity = game.getAgent<Entity>(globalID)) {
			if (entity->isPlayer())
				INFO("Destroying player " << entity->getGID());
			entity->destroy();
		} else
			WARN("DestroyEntityPacket: couldn't find entity " << globalID << '.');
	}
}
