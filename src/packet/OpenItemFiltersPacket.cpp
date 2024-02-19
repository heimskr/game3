#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "packet/OpenItemFiltersPacket.h"
#include "types/DirectedPlace.h"
#include "ui/MainWindow.h"
#include "ui/module/ItemFilterModule.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	void OpenItemFiltersPacket::handle(ClientGame &game) {
		RealmPtr realm = game.getRealm(realmID);
		if (!realm) {
			ERROR_("Couldn't find realm " << realmID << " in OpenItemFiltersPacket handler");
			return;
		}

		MainWindow &window = game.getWindow();

		if (removeOnMove) {
			game.getPlayer()->queueForMove([&window, tab = window.inventoryTab](const auto &, bool) {
				window.queue([tab] {
					tab->removeModule();
				});
				return true;
			});
		}

		// Force a fresh module construction instead of an update
		window.inventoryTab->removeModule();

		DirectedPlace place{direction, Place(position, realm, {})};

		window.queue([&window, place = std::move(place)] {
			window.openModule(ItemFilterModule::ID(), std::any(place));
		});
	}
}
