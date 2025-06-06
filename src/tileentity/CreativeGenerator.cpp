#include "util/Log.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/ServerInventory.h"
#include "realm/Realm.h"
#include "tileentity/CreativeGenerator.h"

namespace Game3 {
	namespace {
		constexpr EnergyAmount ENERGY_CAPACITY = 1'000'000;
		constexpr std::chrono::milliseconds PERIOD{50};
	}

	CreativeGenerator::CreativeGenerator():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	CreativeGenerator::CreativeGenerator(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	CreativeGenerator::CreativeGenerator(Position position_):
		CreativeGenerator("base:tile/creative_generator"_id, position_) {}

	EnergyAmount CreativeGenerator::getEnergyCapacity() {
		assert(energyContainer);
		auto lock = energyContainer->sharedLock();
		return energyContainer->capacity;
	}

	void CreativeGenerator::init(Game &game) {
		TileEntity::init(game);
	}

	void CreativeGenerator::tick(const TickArgs &args) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server)
			return;

		Ticker ticker{*this, args};

		{
			assert(energyContainer);
			auto lock = energyContainer->uniqueLock();
			energyContainer->energy = ENERGY_CAPACITY;
		}

		enqueueTick(PERIOD);
	}

	void CreativeGenerator::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
	}

	bool CreativeGenerator::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		if (modifiers.onlyAlt()) {
			RealmPtr realm = getRealm();
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/creative_generator"_id));
			return true;
		}

		return false;
	}

	void CreativeGenerator::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
	}

	void CreativeGenerator::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
	}

	void CreativeGenerator::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
	}

	void CreativeGenerator::broadcast(bool force) {
		TileEntity::broadcast(force);
	}
}
