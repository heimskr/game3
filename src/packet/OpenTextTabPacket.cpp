#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "packet/OpenTextTabPacket.h"
#include "ui/MainWindow.h"
#include "ui/gl/module/TextModule.h"
#include "ui/tab/GTKInventoryTab.h"
#include "ui/tab/GTKTextTab.h"

namespace Game3 {
	void OpenTextTabPacket::handle(const ClientGamePtr &game) {
		MainWindow &window = game->getWindow();

		if (removeOnMove) {
			game->getPlayer()->queueForMove([&window](const auto &, bool) {
				window.queue([&window] {
					window.inventoryTab->show();
				});
				return true;
			});
		}

		window.queue([&window, message = std::move(message)] mutable {
			window.openModule("base:module/text", std::any(std::move(message)));
		});
	}
}
