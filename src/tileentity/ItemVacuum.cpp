#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "packet/UpdateAgentFieldPacket.h"
#include "realm/Realm.h"
#include "tileentity/ItemVacuum.h"
#include "ui/module/RadiusMachineModule.h"
#include "util/ConstexprHash.h"
#include "util/Log.h"

#include <cassert>

namespace Game3 {
	namespace {
		constexpr std::chrono::milliseconds PERIOD{100};
		constexpr EnergyAmount ENERGY_CAPACITY = 100'000;
		constexpr Slot INVENTORY_CAPACITY = 10;
		constexpr EnergyAmount ENERGY_PER_ACTION = 500;
		constexpr Radius DEFAULT_RADIUS = 2;
	}

	ItemVacuum::ItemVacuum():
		EnergeticTileEntity(ENERGY_CAPACITY),
		HasRadius(DEFAULT_RADIUS) {}

	ItemVacuum::ItemVacuum(Identifier tile_id, Position position):
		TileEntity(std::move(tile_id), ID(), position, true),
		EnergeticTileEntity(ENERGY_CAPACITY),
		HasRadius(DEFAULT_RADIUS) {}

	ItemVacuum::ItemVacuum(Position position):
		ItemVacuum("base:tile/item_vacuum"_id, position) {}

	EnergyAmount ItemVacuum::getEnergyCapacity() {
		assert(energyContainer);
		auto lock = energyContainer->sharedLock();
		return energyContainer->capacity;
	}

	void ItemVacuum::init(Game &game) {
		TileEntity::init(game);
		HasInventory::setInventory(Inventory::create(shared_from_this(), INVENTORY_CAPACITY), 0);
	}

	void ItemVacuum::tick(const TickArgs &args) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server) {
			return;
		}

		Ticker ticker{*this, args};

		enqueueTick(PERIOD);

		InventoryPtr inventory = getInventory(0);
		if (!inventory->hasOwner()) {
			inventory->setOwner(weak_from_this());
		}

		scan();
	}

	void ItemVacuum::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
	}

	bool ItemVacuum::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		if (getSide() == Side::Client)
			return false;


		if (modifiers.onlyAlt()) {
			{
				const InventoryPtr inventory = getInventory(0);
				auto lock = inventory->sharedLock();
				inventory->iterate([&](const ItemStackPtr &stack, Slot) {
					player->give(stack);
					return false;
				});
			}
			RealmPtr realm = getRealm();
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/ItemVacuum"_id));
			return true;
		}

		player->send(make<OpenModuleForAgentPacket>(RadiusMachineModule::ID(), getGID()));
		InventoriedTileEntity::addObserver(player, true);
		EnergeticTileEntity::addObserver(player, true);

		return true;
	}

	void ItemVacuum::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
	}

	void ItemVacuum::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
		buffer << radius;
	}

	void ItemVacuum::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
		buffer >> radius;
	}

	bool ItemVacuum::setField(uint32_t field_name, Buffer &field_value, const PlayerPtr &updater) {
		switch (field_name) {
			AGENT_FIELD(radius, true);
			default:
				return TileEntity::setField(field_name, field_value, updater);
		}
	}

	void ItemVacuum::broadcast(bool force) {
		assert(getSide() == Side::Server);

		if (force) {
			TileEntity::broadcast(true);
			return;
		}

		const auto packet = make<TileEntityPacket>(getSelf());
		const auto energy_packet = makeEnergyPacket();

		{
			auto energetic_lock = EnergeticTileEntity::observers.uniqueLock();
			std::erase_if(EnergeticTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
				if (auto player = weak_player.lock()) {
					player->send(energy_packet);
					return false;
				}

				return true;
			});
		}

		auto inventoried_lock = InventoriedTileEntity::observers.uniqueLock();

		std::erase_if(InventoriedTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				player->send(packet);
				return false;
			}

			return true;
		});
	}

	GamePtr ItemVacuum::getGame() const {
		return TileEntity::getGame();
	}

	bool ItemVacuum::scan() {
		const InventoryPtr inventory = getInventory(0);

		assert(inventory);
		assert(energyContainer);

		auto energy_lock = energyContainer->uniqueLock();
		if (energyContainer->energy < ENERGY_PER_ACTION) {
			return false;
		}

		GamePtr game = getGame();
		Position center = getPosition();

		auto item = std::static_pointer_cast<ItemEntity>(getRealm()->findEntitySquare(center, radius, [](const EntityPtr &entity) {
			return std::dynamic_pointer_cast<ItemEntity>(entity) != nullptr;
		}));

		if (!item) {
			return false;
		}

		auto inventory_lock = inventory->uniqueLock();

		if (ItemStackPtr leftover = inventory->add(item->getStack())) {
			if (leftover->count != item->getStack()->count) {
				item->setStack(leftover);
				item->increaseUpdateCounter();
			}
		} else {
			item->remove();
		}

		energyContainer->remove(ENERGY_PER_ACTION);
		return true;
	}
}
