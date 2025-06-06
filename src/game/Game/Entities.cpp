#include "entity/Blacksmith.h"
#include "entity/Bomb.h"
#include "entity/Chicken.h"
#include "entity/Crab.h"
#include "entity/Cyclops.h"
#include "entity/Dog.h"
#include "entity/Egg.h"
#include "entity/EntityFactory.h"
#include "entity/ExplosionParticle.h"
#include "entity/Eye.h"
#include "entity/FluidParticle.h"
#include "entity/ItemEntity.h"
#include "entity/Merchant.h"
#include "entity/Miner.h"
#include "entity/Pig.h"
#include "entity/Player.h"
#include "entity/Quarter.h"
#include "entity/Sheep.h"
#include "entity/Ship.h"
#include "entity/Snowball.h"
#include "entity/SquareParticle.h"
#include "entity/TextParticle.h"
#include "entity/Woodcutter.h"
#include "entity/Worker.h"
#include "game/Game.h"

namespace Game3 {
	void Game::add(EntityFactory &&factory) {
		auto shared = std::make_shared<EntityFactory>(std::move(factory));
		registry<EntityFactoryRegistry>().add(shared->identifier, shared);
	}

	void Game::addEntityFactories() {
		auto &reg = registry<EntityFactoryRegistry>();

		auto add = [&](auto &&factory) {
			auto shared = std::make_shared<EntityFactory>(std::forward<decltype(factory)>(factory));
			reg.add(shared->identifier, shared);
		};

		add(EntityFactory::create<Blacksmith>());
		add(EntityFactory::create<Bomb>());
		add(EntityFactory::create<Chicken>());
		add(EntityFactory::create<Crab>());
		add(EntityFactory::create<Cyclops>());
		add(EntityFactory::create<Dog>());
		add(EntityFactory::create<Egg>());
		add(EntityFactory::create<ExplosionParticle>());
		add(EntityFactory::create<Eye>());
		add(EntityFactory::create<FluidParticle>());
		add(EntityFactory::create<ItemEntity>());
		add(EntityFactory::create<Merchant>());
		add(EntityFactory::create<Miner>());
		add(EntityFactory::create<Pig>());
		add(EntityFactory::create<Quarter>());
		add(EntityFactory::create<Sheep>());
		add(EntityFactory::create<Ship>());
		add(EntityFactory::create<Snowball>());
		add(EntityFactory::create<SquareParticle>());
		add(EntityFactory::create<TextParticle>());
		add(EntityFactory::create<Woodcutter>());
	}
}
