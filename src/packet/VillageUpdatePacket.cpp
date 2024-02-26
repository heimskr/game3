#include "Log.h"
#include "game/ClientGame.h"
#include "game/Village.h"
#include "packet/VillageUpdatePacket.h"
#include "ui/MainWindow.h"

namespace Game3 {
	VillageUpdatePacket::VillageUpdatePacket(const Village &village):
		VillageUpdatePacket(village.getID(), village.getRealmID(), village.getChunkPosition(), village.getPosition(), village.getName(), village.getLabor(), village.getGreed(), village.getResources()) {}

	void VillageUpdatePacket::handle(const ClientGamePtr &game) {
		VillagePtr village;

		try {
			village = game->getVillage(villageID);
		} catch (const std::out_of_range &) {
			RealmPtr realm;
			try {
				realm = game->getRealm(realmID);
			} catch (const std::out_of_range &) {
				WARN("Couldn't find realm {} when handling village update packet for village {}", realmID, villageID);
				game->iterateRealms([](const RealmPtr &realm) {
					INFO("Realm: {}", realm->getID());
				});
				return;
			}

			village = game->addVillage(*game, villageID, name, realmID, chunkPosition, position);
			INFO("Added new village {}", *village);
		}

		village->setResources(std::move(resources));
		village->setLabor(labor);
		village->setGreed(greed);
		game->signalVillageUpdate().emit(village);
	}
}
