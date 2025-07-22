#include "util/Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "packet/DisplayTextPacket.h"
#include "ui/module/TextModule.h"
#include "ui/GameUI.h"
#include "ui/Window.h"

namespace Game3 {
	void DisplayTextPacket::handle(const ClientGamePtr &game) {
		auto window = game->getWindow();

		if (removeOnMove) {
			game->getPlayer()->queueForMove([window](const auto &, bool) {
				window->queue([](Window &window) {
					if (auto game_ui = window.uiContext.getUI<GameUI>()) {
						game_ui->removeModule();
						game_ui->hideOmniDialog();
					}
				});
				return true;
			});
		}

		window->queue([message = std::move(message)](Window &window) mutable {
			if (auto game_ui = window.uiContext.getUI<GameUI>()) {
				game_ui->openModule("base:module/text", std::any(std::move(message)));
			}
		});
	}
}
