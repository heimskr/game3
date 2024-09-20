#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "packet/OpenItemFiltersPacket.h"
#include "types/DirectedPlace.h"
#include "ui/MainWindow.h"
#include "ui/gl/module/ItemFiltersModule.h"

namespace Game3 {
	void OpenItemFiltersPacket::handle(const ClientGamePtr &game) {
		RealmPtr realm = game->getRealm(realmID);
		if (!realm) {
			ERROR("Couldn't find realm {} in OpenItemFiltersPacket handler", realmID);
			return;
		}

		MainWindow &window = game->getWindow();

		if (removeOnMove) {
			game->getPlayer()->queueForMove([&window](const auto &, bool) {
				window.queue([&window] {
					window.removeModule();
					window.closeOmniDialog();
				});
				return true;
			});
		}

		// Force a fresh module construction instead of an update
		window.removeModule();

		DirectedPlace place{direction, Place(position, realm, {})};

		window.queue([&window, place = std::move(place)] {
			window.openModule(ItemFiltersModule::ID(), std::any(place));
		});
	}
}
