#include "game/Game.h"
#include "tileentity/Building.h"
#include "tileentity/Centrifuge.h"
#include "tileentity/ChemicalReactor.h"
#include "tileentity/Chest.h"
#include "tileentity/Combiner.h"
#include "tileentity/CraftingStation.h"
#include "tileentity/Dissolver.h"
#include "tileentity/GeothermalGenerator.h"
#include "tileentity/ItemSpawner.h"
#include "tileentity/OreDeposit.h"
#include "tileentity/Pipe.h"
#include "tileentity/Pump.h"
#include "tileentity/Sign.h"
#include "tileentity/Stockpile.h"
#include "tileentity/Tank.h"
#include "tileentity/Teleporter.h"
#include "tileentity/TileEntity.h"
#include "tileentity/TileEntityFactory.h"

namespace Game3 {
	void Game::add(TileEntityFactory &&factory) {
		auto shared = std::make_shared<TileEntityFactory>(std::move(factory));
		registry<TileEntityFactoryRegistry>().add(shared->identifier, shared);
	}

	void Game::addTileEntityFactories() {
		add(TileEntityFactory::create<Building>());
		add(TileEntityFactory::create<Centrifuge>());
		add(TileEntityFactory::create<ChemicalReactor>());
		add(TileEntityFactory::create<Chest>());
		add(TileEntityFactory::create<Combiner>());
		add(TileEntityFactory::create<CraftingStation>());
		add(TileEntityFactory::create<Dissolver>());
		add(TileEntityFactory::create<GeothermalGenerator>());
		add(TileEntityFactory::create<ItemSpawner>());
		add(TileEntityFactory::create<OreDeposit>());
		add(TileEntityFactory::create<Pipe>());
		add(TileEntityFactory::create<Pump>());
		add(TileEntityFactory::create<Sign>());
		add(TileEntityFactory::create<Stockpile>());
		add(TileEntityFactory::create<Tank>());
		add(TileEntityFactory::create<Teleporter>());
	}
}
