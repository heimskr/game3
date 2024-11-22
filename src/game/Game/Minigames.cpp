#include "game/Game.h"
#include "minigame/Breakout.h"
#include "minigame/FlappyBird.h"
#include "minigame/MathGame.h"
#include "minigame/Minigame.h"
#include "minigame/MinigameFactory.h"

namespace Game3 {
	void Game::addMinigameFactories() {
		add(MinigameFactory::create<Breakout>());
		add(MinigameFactory::create<MathGame>());
#ifdef ENABLE_ZIP8
		add(MinigameFactory::create<FlappyBird>());
#endif
	}

	void Game::add(MinigameFactory &&factory) {
		auto shared = std::make_shared<MinigameFactory>(std::move(factory));
		registry<MinigameFactoryRegistry>().add(shared->identifier, shared);
	}
}
