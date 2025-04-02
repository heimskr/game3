#include "game/ClientGame.h"
#include "minigame/MinigameFactory.h"
#include "packet/OpenMinigamePacket.h"
#include "ui/gl/dialog/MinigameDialog.h"
#include "ui/Window.h"

namespace Game3 {
	OpenMinigamePacket::OpenMinigamePacket(Identifier minigame_id, int game_width, int game_height):
		minigameID(std::move(minigame_id)),
		gameWidth(game_width),
		gameHeight(game_height) {}

	void OpenMinigamePacket::handle(const std::shared_ptr<ClientGame> &game) {
		game->getWindow()->queue([game, id = minigameID, width = gameWidth, height = gameHeight](Window &window) {
			auto &registry = game->registry<MinigameFactoryRegistry>();
			if (auto factory = registry.maybe(id)) {
				window.uiContext.emplaceDialog<MinigameDialog>(1, (*factory)(game, {}), width, height);
			} else {
				window.error(std::format("Unknown minigame: {}", id));
			}
		});
	}
}
