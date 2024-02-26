#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "packet/EntityMoneyChangedPacket.h"

namespace Game3 {
	EntityMoneyChangedPacket::EntityMoneyChangedPacket(const Entity &entity):
		EntityMoneyChangedPacket(entity.getGID(), entity.getMoney()) {}

	void EntityMoneyChangedPacket::handle(const ClientGamePtr &game) {
		EntityPtr entity = game->getAgent<Entity>(globalID);
		if (!entity) {
			WARN("Couldn't find entity {} while trying to set money to {}", globalID, moneyCount);
			return;
		}

		entity->setMoney(moneyCount);
	}
}
