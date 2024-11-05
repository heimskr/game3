#include "game/ClientGame.h"
#include "minigame/MinigameFactory.h"
#include "ui/Window.h"

namespace Game3 {
	MinigameFactory::MinigameFactory(Identifier identifier, decltype(function) function):
		NamedRegisterable(std::move(identifier)), function(std::move(function)) {}

	std::shared_ptr<Minigame> MinigameFactory::operator()(const std::shared_ptr<ClientGame> &game, const std::any &argument) const {
		if (!function) {
			throw std::logic_error("MinigameFactory is missing a function");
		}

		return function(game, argument);
	}

	UIContext & MinigameFactory::getUIContext(ClientGame &game) {
		return game.getUIContext();
	}
}
