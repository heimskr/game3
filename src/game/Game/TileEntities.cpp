#include "game/Game.h"
#include "tileentity/ArcadeMachine.h"
#include "tileentity/Autocrafter.h"
#include "tileentity/Autofarmer.h"
#include "tileentity/BiomassLiquefier.h"
#include "tileentity/Building.h"
#include "tileentity/Centrifuge.h"
#include "tileentity/ChemicalReactor.h"
#include "tileentity/Chest.h"
#include "tileentity/Combiner.h"
#include "tileentity/Computer.h"
#include "tileentity/CraftingStation.h"
#include "tileentity/Crate.h"
#include "tileentity/CreativeGenerator.h"
#include "tileentity/Disruptor.h"
#include "tileentity/Dissolver.h"
#include "tileentity/EntityBuilding.h"
#include "tileentity/EternalFountain.h"
#include "tileentity/GeothermalGenerator.h"
#include "tileentity/Incinerator.h"
#include "tileentity/Incubator.h"
#include "tileentity/ItemSpawner.h"
#include "tileentity/Lamp.h"
#include "tileentity/Liquefier.h"
#include "tileentity/Microscope.h"
#include "tileentity/Mutator.h"
#include "tileentity/OreDeposit.h"
#include "tileentity/Pipe.h"
#include "tileentity/PressurePlate.h"
#include "tileentity/Pump.h"
#include "tileentity/Recombinator.h"
#include "tileentity/Sequencer.h"
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
		add(TileEntityFactory::create<ArcadeMachine>());
		add(TileEntityFactory::create<Autocrafter>());
		add(TileEntityFactory::create<Autofarmer>());
		add(TileEntityFactory::create<BiomassLiquefier>());
		add(TileEntityFactory::create<Building>());
		add(TileEntityFactory::create<Centrifuge>());
		add(TileEntityFactory::create<ChemicalReactor>());
		add(TileEntityFactory::create<Chest>());
		add(TileEntityFactory::create<Combiner>());
#ifdef GAME3_ENABLE_SCRIPTING
		add(TileEntityFactory::create<Computer>());
#endif
		add(TileEntityFactory::create<CraftingStation>());
		add(TileEntityFactory::create<Crate>());
		add(TileEntityFactory::create<CreativeGenerator>());
		add(TileEntityFactory::create<Disruptor>());
		add(TileEntityFactory::create<Dissolver>());
		add(TileEntityFactory::create<EntityBuilding>());
		add(TileEntityFactory::create<EternalFountain>());
		add(TileEntityFactory::create<GeothermalGenerator>());
		add(TileEntityFactory::create<Incinerator>());
		add(TileEntityFactory::create<Incubator>());
		add(TileEntityFactory::create<ItemSpawner>());
		add(TileEntityFactory::create<Lamp>());
		add(TileEntityFactory::create<Liquefier>());
		add(TileEntityFactory::create<Microscope>());
		add(TileEntityFactory::create<Mutator>());
		add(TileEntityFactory::create<OreDeposit>());
		add(TileEntityFactory::create<Pipe>());
		add(TileEntityFactory::create<PressurePlate>());
		add(TileEntityFactory::create<Pump>());
		add(TileEntityFactory::create<Recombinator>());
		add(TileEntityFactory::create<Sequencer>());
		add(TileEntityFactory::create<Sign>());
		add(TileEntityFactory::create<Stockpile>());
		add(TileEntityFactory::create<Tank>());
		add(TileEntityFactory::create<Teleporter>());
	}
}
