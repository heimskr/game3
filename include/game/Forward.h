#pragma once

#include <memory>

namespace Game3 {
	class Agent;
	using AgentPtr = std::shared_ptr<Agent>;

	class ClientGame;
	using ClientGamePtr = std::shared_ptr<ClientGame>;

	class ClientInventory;
	using ClientInventoryPtr = std::shared_ptr<ClientInventory>;

	class Container;
	using ContainerPtr = std::shared_ptr<Container>;

	class Crop;
	using CropPtr = std::shared_ptr<Crop>;

	class EnergyContainer;
	using EnergyContainerPtr = std::shared_ptr<EnergyContainer>;

	class FluidContainer;
	using FluidContainerPtr = std::shared_ptr<FluidContainer>;

	class Game;
	using GamePtr = std::shared_ptr<Game>;

	class Inventory;
	using InventoryPtr = std::shared_ptr<Inventory>;

	class Observable;
	using ObservablePtr = std::shared_ptr<Observable>;

	class Resource;
	using ResourcePtr = std::shared_ptr<Resource>;

	class ServerGame;
	using ServerGamePtr = std::shared_ptr<ServerGame>;

	class ServerInventory;
	using ServerInventoryPtr = std::shared_ptr<ServerInventory>;

	class Village;
	using VillagePtr = std::shared_ptr<Village>;
}
