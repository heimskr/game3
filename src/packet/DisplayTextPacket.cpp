#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "packet/DisplayTextPacket.h"
#include "ui/MainWindow.h"
#include "ui/gl/module/TextModule.h"

namespace Game3 {
	void DisplayTextPacket::handle(const ClientGamePtr &game) {
		MainWindow &window = game->getWindow();

		if (removeOnMove) {
			game->getPlayer()->queueForMove([&window](const auto &, bool) {
				window.queue([&window] {
					window.removeModule();
				});
				return true;
			});
		}

		window.queue([&window, message = std::move(message)] mutable {
			window.openModule("base:module/text", std::any(std::move(message)));
		});
	}
}
