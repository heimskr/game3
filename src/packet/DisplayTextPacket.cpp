#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "packet/DisplayTextPacket.h"
#include "ui/Window.h"
#include "ui/gl/module/TextModule.h"

namespace Game3 {
	void DisplayTextPacket::handle(const ClientGamePtr &game) {
		auto window = game->getWindow();

		if (removeOnMove) {
			game->getPlayer()->queueForMove([window](const auto &, bool) {
				window->queue([](Window &window) {
					window.removeModule();
					window.closeOmniDialog();
				});
				return true;
			});
		}

		window->queue([message = std::move(message)](Window &window) mutable {
			window.openModule("base:module/text", std::any(std::move(message)));
		});
	}
}
