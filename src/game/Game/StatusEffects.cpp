#include "game/Game.h"
#include "statuseffect/Burning.h"
#include "statuseffect/StatusEffectFactory.h"

namespace Game3 {
	void Game::addStatusEffectFactories() {
		add(StatusEffectFactory::create<Burning>());
	}

	void Game::add(StatusEffectFactory &&factory) {
		auto shared = std::make_shared<StatusEffectFactory>(std::move(factory));
		registry<StatusEffectFactoryRegistry>().add(shared->identifier, shared);
	}
}
