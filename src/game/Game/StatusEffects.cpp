#include "game/Game.h"
#include "statuseffect/Burning.h"
#include "statuseffect/Chilling.h"
#include "statuseffect/Irradiated.h"
#include "statuseffect/Pickled.h"
#include "statuseffect/StatusEffectFactory.h"

namespace Game3 {
	void Game::addStatusEffectFactories() {
		add(StatusEffectFactory::create<Burning>());
		add(StatusEffectFactory::create<Chilling>());
		add(StatusEffectFactory::create<Irradiated>());
		add(StatusEffectFactory::create<Pickled>());
	}

	void Game::add(StatusEffectFactory &&factory) {
		auto shared = std::make_shared<StatusEffectFactory>(std::move(factory));
		registry<StatusEffectFactoryRegistry>().add(shared->identifier, shared);
	}
}
