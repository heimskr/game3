#include "util/Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "packet/OpenItemFiltersPacket.h"
#include "types/DirectedPlace.h"
#include "ui/Window.h"
#include "ui/gl/module/ItemFiltersModule.h"

namespace Game3 {
	void OpenItemFiltersPacket::handle(const ClientGamePtr &game) {
		RealmPtr realm = game->getRealm(realmID);
		if (!realm) {
			ERR("Couldn't find realm {} in OpenItemFiltersPacket handler", realmID);
			return;
		}

		auto window = game->getWindow();

		if (removeOnMove) {
			game->getPlayer()->queueForMove([window](const auto &, bool) {
				window->queue([](Window &window) {
					window.removeModule();
					window.hideOmniDialog();
				});
				return true;
			});
		}

		// Force a fresh module construction instead of an update
		window->removeModule();

		DirectedPlace place{direction, Place(position, realm, {})};

		window->queue([place = std::move(place)](Window &window) {
			window.openModule(ItemFiltersModule::ID(), std::any(place));
		});
	}
}
