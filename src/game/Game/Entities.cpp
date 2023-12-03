#include "game/Game.h"
#include "entity/Blacksmith.h"
#include "entity/Chicken.h"
#include "entity/Dog.h"
#include "entity/EntityFactory.h"
#include "entity/ItemEntity.h"
#include "entity/Merchant.h"
#include "entity/Miner.h"
#include "entity/Pig.h"
#include "entity/Player.h"
#include "entity/Sheep.h"
#include "entity/Woodcutter.h"
#include "entity/Worker.h"
#include "entity/Cyclops.h"

namespace Game3 {
	void Game::add(EntityFactory &&factory) {
		auto shared = std::make_shared<EntityFactory>(std::move(factory));
		registry<EntityFactoryRegistry>().add(shared->identifier, shared);
	}

	void Game::addEntityFactories() {
		add(EntityFactory::create<Blacksmith>());
		add(EntityFactory::create<Chicken>());
		add(EntityFactory::create<Dog>());
		add(EntityFactory::create<ItemEntity>());
		add(EntityFactory::create<Merchant>());
		add(EntityFactory::create<Miner>());
		add(EntityFactory::create<Pig>());
		add(EntityFactory::create<Sheep>());
		add(EntityFactory::create<Woodcutter>());
		add(EntityFactory::create<Cyclops>());
	}
}
