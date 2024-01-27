#include "Log.h"
#include "game/ClientGame.h"
#include "game/Village.h"
#include "packet/VillageUpdatePacket.h"
#include "ui/MainWindow.h"

namespace Game3 {
	VillageUpdatePacket::VillageUpdatePacket(const Village &village):
		VillageUpdatePacket(village.getID(), village.getRealmID(), village.getChunkPosition(), village.getPosition(), village.getName(), village.getLabor(), village.getResources()) {}

	void VillageUpdatePacket::handle(ClientGame &game) {
		VillagePtr village;

		try {
			village = game.getVillage(villageID);
		} catch (const std::out_of_range &) {
			RealmPtr realm;
			try {
				realm = game.getRealm(realmID);
			} catch (const std::out_of_range &) {
				WARN_("Couldn't find realm " << realmID << " when handling village update packet for village " << villageID);
				game.iterateRealms([](const RealmPtr &realm) {
					INFO_("Realm: " << realm->getID());
				});
				return;
			}

			village = game.addVillage(game, villageID, name, realmID, chunkPosition, position);
			INFO("Added new village {}", *village);
		}

		village->setResources(std::move(resources));
		village->setLabor(labor);
		game.signalVillageUpdate().emit(village);
	}
}
